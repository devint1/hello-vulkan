/*
 * This file is part of Hello Vulkan.
 *
 * Hello Vulkan is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hello Vulkan is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hello Vulkan.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libgen.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "config.h"
#include "maths.h"
#include "scene.h"

#define VK_CHECK_ERROR(x) if(!(x)) return 0
#define VK_DESTROY(device, object, function) if(device && object) \
	function(device, object, NULL)

const static char* const REQUIRED_EXTENSION = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
const static char* const SHADER_SEARCH_PATHS[] = {
	"./src/shaders/",
	"./shaders/"
};
const static char* const TEXTURE_SEARCH_PATHS[] = {
	"./textures/",
	"../textures/"
};
const static char* const INSTALL_DATA_SEARCH_PATH = "/../share/" PACKAGE "/";

#pragma pack(0)
typedef struct _TexHdr {
	uint32_t width, height;
	uint32_t format, type;
} TexHdr;

static VkInstance createInstance() {
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Vulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	const char* const enabledLayerNames[] = { "VK_LAYER_LUNARG_standard_validation" };
	createInfo.enabledLayerCount = 1;
	createInfo.ppEnabledLayerNames = enabledLayerNames;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	VkInstance instance;
	if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
		fprintf(stderr, "Could not create Vulkan instance.\n");
		return NULL;
	}
	return instance;
}

static int findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties *queueFamilies =
		malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
		queueFamilies);

	for (uint32_t i = 0; i < queueFamilyCount; ++i) {
		VkBool32 presentSupport = 0;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
			&presentSupport);
		VkQueueFamilyProperties queueFamily = queueFamilies[i];
		if (queueFamily.queueCount > 0 && presentSupport
			&& queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			free(queueFamilies);
			return i;
		}
	}
	free(queueFamilies);
	return -1;
}

static int checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

	VkExtensionProperties *availableExtensions =
		malloc(sizeof(VkExtensionProperties) * extensionCount);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount,
		availableExtensions);

	for (uint32_t i = 0; i < extensionCount; ++i) {
		VkExtensionProperties extension = availableExtensions[i];
		if (!strcmp(REQUIRED_EXTENSION, extension.extensionName)) {
			free(availableExtensions);
			return 1;
		}
	}

	free(availableExtensions);
	return 0;
}

static int querySwapChainSupport(VkPhysicalDevice device,
	VkSurfaceKHR surface, uint32_t *formatCount, uint32_t *presentModeCount,
	VkSurfaceFormatKHR **formats, VkPresentModeKHR **presentModes) {

	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, formatCount, NULL);

	if (formats != NULL) {
		*formats = NULL;
		if (*formatCount != 0) {
			*formats =
				malloc(sizeof(VkSurfaceFormatKHR) * *formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, formatCount,
				*formats);
		}
	}

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		presentModeCount, NULL);

	if (presentModes != NULL) {
		*presentModes = NULL;
		if (*presentModeCount != 0) {
			*presentModes =
				malloc(sizeof(VkPresentModeKHR) * *presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
				presentModeCount, *presentModes);
		}
	}

	return *formatCount > 0 && *presentModeCount > 0;
}

static int rateDeviceSuitability(VkPhysicalDevice device,
	VkSurfaceKHR surface) {

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int score = 0;

	// Application can't function without geometry shaders
	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	uint32_t formatCount, presentModeCount;
	if (!querySwapChainSupport(device, surface, &formatCount, &presentModeCount,
		NULL, NULL)) {
		return 0;
	}

	if (!checkDeviceExtensionSupport(device)) {
		return 0;
	}

	if (findQueueFamilies(device, surface) == -1) {
		return 0;
	}

	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score += 1000;
	}

	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	return score;
}

static VkPhysicalDevice pickPhysicalDevice(const VkContext *const context) {

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount;
	vkEnumeratePhysicalDevices(context->instance, &deviceCount, NULL);
	if (deviceCount == 0) {
		fprintf(stderr, "No GPUs with Vulkan support found.\n");
		return VK_NULL_HANDLE;
	}
	VkPhysicalDevice *devices =
		malloc(sizeof(VkPhysicalDevice) * deviceCount);
	vkEnumeratePhysicalDevices(context->instance, &deviceCount, devices);
	int maxScore = 0;
	for (uint32_t i = 0; i < deviceCount; ++i) {
		int score = rateDeviceSuitability(devices[i], context->surface);
		if (score > maxScore) {
			physicalDevice = devices[i];
			maxScore = score;
		}
	}
	if (physicalDevice == VK_NULL_HANDLE) {
		fprintf(stderr, "Failed to find a suitable GPU.\n");
	}
	free(devices);
	return physicalDevice;
}

static VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow *window) {
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
		fprintf(stderr, "Could not create Vulkan surface.\n");
		return NULL;
	}
	return surface;
}

static VkDevice createDevice(const VkContext* const context,
							 int *queueFamilyIndex) {

	*queueFamilyIndex = findQueueFamilies(context->physicalDevice,
		context->surface);

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = *queueFamilyIndex;
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = &REQUIRED_EXTENSION;

	VkDevice device;
	if (vkCreateDevice(context->physicalDevice, &createInfo, NULL, &device) != VK_SUCCESS) {
		fprintf(stderr, "Could not create logical device.\n");
		return NULL;
	}
	return device;
}

static VkQueue getGraphicsQueue(VkDevice device, int queueFamilyIndex) {
	VkQueue graphicsQueue;
	vkGetDeviceQueue(device, queueFamilyIndex, 0, &graphicsQueue);
	return graphicsQueue;
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(uint32_t formatCount,
	const VkSurfaceFormatKHR* const formats) {

	if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
		return (VkSurfaceFormatKHR)
			{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}

	for (uint32_t i = 0; i < formatCount; ++i) {
		VkSurfaceFormatKHR format = formats[i];
		if (format.format == VK_FORMAT_B8G8R8A8_UNORM
			&& format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return format;
		}
	}

	return formats[0];
}

static VkPresentModeKHR chooseSwapPresentMode(uint32_t presentModeCount,
											  const VkPresentModeKHR* const presentModes,
											  int vsync) {

	if (vsync) {
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkPresentModeKHR bestMode = VK_PRESENT_MODE_IMMEDIATE_KHR;

	for (uint32_t i = 0; i < presentModeCount; ++i) {
		VkPresentModeKHR presentMode = presentModes[i];
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return presentMode;
		}
	}

	return bestMode;
}

static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR* const capabilities,
								   uint32_t width, uint32_t height) {

	if (capabilities->currentExtent.width != UINT_MAX) {
		return capabilities->currentExtent;
	} else {
		VkExtent2D actualExtent = { width, height };

		actualExtent.width = MAX(capabilities->minImageExtent.width,
			MIN(capabilities->maxImageExtent.width, actualExtent.width));
		actualExtent.height = MAX(capabilities->minImageExtent.height,
			MIN(capabilities->maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

static int createSwapChain(VkContext* context, uint32_t width, uint32_t height,
						   int vsync) {

	uint32_t formatCount, presentModeCount;
	VkSurfaceFormatKHR *formats;
	VkPresentModeKHR *presentModes;
	querySwapChainSupport(context->physicalDevice, context->surface,
		&formatCount, &presentModeCount, &formats, &presentModes);
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(context->physicalDevice,
		context->surface, &capabilities);

	context->surfaceFormat = chooseSwapSurfaceFormat(formatCount,
		formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(presentModeCount,
		presentModes, vsync);
	context->extent = chooseSwapExtent(&capabilities, width, height);

	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = context->surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = context->surfaceFormat.format;
	createInfo.imageColorSpace = context->surfaceFormat.colorSpace;
	createInfo.imageExtent = context->extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	int queueIndex = findQueueFamilies(context->physicalDevice,
		context->surface);
	uint32_t queueFamilyIndices[] = { queueIndex, queueIndex };
	createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
	createInfo.queueFamilyIndexCount = 2;
	createInfo.pQueueFamilyIndices = queueFamilyIndices;

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(context->device, &createInfo, NULL,
							 &context->swapChain) != VK_SUCCESS) {

		fprintf(stderr, "Failed to create swap chain.\n");
		free(formats);
		free(presentModes);
		return 0;
	}

	free(formats);
	free(presentModes);
	return 1;
}

static VkImageView createImageView(const VkContext* const context,
								   VkImage image, VkFormat format,
								   VkImageAspectFlags aspectFlags,
								   uint32_t layerCount) {

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY
		: VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = layerCount;

	VkImageView textureImageView;
	if (vkCreateImageView(context->device, &viewInfo, NULL, &textureImageView) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create texture image view.\n");
		return NULL;
	}
	return textureImageView;
}

static int createImageViews(VkContext* context) {
	if (vkGetSwapchainImagesKHR(context->device, context->swapChain,
								&context->imageViewCount, NULL) != VK_SUCCESS) {
		return 0;
	}
	VkImage *swapChainImages = malloc(context->imageViewCount * sizeof(VkImage));
	context->swapChainImageViews =
		malloc(context->imageViewCount * sizeof(VkImageView));
	if (vkGetSwapchainImagesKHR(context->device, context->swapChain,
								&context->imageViewCount,
								swapChainImages) != VK_SUCCESS) {
		return 0;
	};

	for (uint32_t i = 0; i < context->imageViewCount; i++) {
		context->swapChainImageViews[i] = createImageView(context,
			swapChainImages[i], context->surfaceFormat.format,
			VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}

	free(swapChainImages);
	return 1;
}

static void warnPath(const char* const format, ...) {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "WARNING: Path exceeds maximum length; skipping: ");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
}

static FILE* openFile(const char* const fileName,
	const char* const *searchPaths, size_t numSearchPaths) {

	char filePath[PATH_MAX], exePath[PATH_MAX];
	char *exeDir;
	FILE *file = NULL;

	// Search datadir in the install path
	readlink("/proc/self/exe", exePath, PATH_MAX);
	exeDir = dirname(exePath);
	if (strlen(exeDir) + strlen(fileName)
		+ strlen(INSTALL_DATA_SEARCH_PATH) < PATH_MAX) {

		sprintf(filePath, "%s%s%s", exeDir, INSTALL_DATA_SEARCH_PATH,
			fileName);
		file = fopen(filePath, "r");
	} else {
		warnPath("%s%s%s", exeDir, INSTALL_DATA_SEARCH_PATH, fileName);
	}

	// Search remaining paths
	if (file == NULL) {
		for (size_t i = 0; i < numSearchPaths; ++i) {
			const char* const searchPath = searchPaths[i];
			if (strlen(searchPath) + strlen(fileName) >= PATH_MAX) {
				warnPath("%s%s", searchPath, fileName);
				continue;
			}
			sprintf(filePath, "%s%s", searchPath, fileName);
			file = fopen(filePath, "r");
			if (file != NULL) {
				break;
			}
		}
	}

	return file;
}

static long readShaderFile(const char* const fileName, uint32_t **destBuffer) {
	size_t numSearchPaths = sizeof(SHADER_SEARCH_PATHS) / sizeof(char*);
	FILE *file = openFile(fileName, SHADER_SEARCH_PATHS, numSearchPaths);

	if (file == NULL) {
		fprintf(stderr, "Failed to locate and open file: %s\n", fileName);
		return 0;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);
	*destBuffer = malloc(length);
	fread(*destBuffer, sizeof(uint32_t), length / sizeof(uint32_t), file);
	fclose(file);

	return length;
}

static VkShaderModule createShaderModule(VkDevice device,
	const uint32_t* const code, long codeSize) {

	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = codeSize;
	createInfo.pCode = code;

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create shader module.\n");
		return NULL;
	}
	return shaderModule;
}

static VkFormat findSupportedFormat(const VkContext* const context,
									const VkFormat* const candidates,
									size_t numCandidates, VkImageTiling tiling,
									VkFormatFeatureFlags features) {

	for (size_t i = 0; i < numCandidates; ++i) {
		VkFormat format = candidates[i];
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(context->physicalDevice, format,
			&props);
		if (tiling == VK_IMAGE_TILING_LINEAR
			&& (props.linearTilingFeatures & features) == features) {

			return format;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL
				   && (props.optimalTilingFeatures & features) == features) {

			return format;
		}
	}

	fprintf(stderr, "Failed to find a supported format.\n");
	return VK_FORMAT_UNDEFINED;
}

static VkFormat findDepthFormat(const VkContext* const context) {
	VkFormat candidates[] = { VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	size_t numCandidates = sizeof(candidates) / sizeof(VkFormat);
    return findSupportedFormat(context, candidates, numCandidates,
		VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

static VkRenderPass createRenderPass(const VkContext* const context) {

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = context->surfaceFormat.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat(context);
	if (depthAttachment.format == VK_FORMAT_UNDEFINED) {
		return NULL;
	}
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
		| VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = sizeof(attachments) / sizeof(VkAttachmentDescription);
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkRenderPass renderPass;
	if (vkCreateRenderPass(context->device, &renderPassInfo, NULL,
						   &renderPass) != VK_SUCCESS) {

		fprintf(stderr, "Failed to create render pass.\n");
		return NULL;
	}

	return renderPass;
}

static VkDescriptorSetLayout createDescriptorSetLayout(const VkContext *const context) {
	VkDescriptorSetLayoutBinding mvpUBOLayoutBinding = {};
	mvpUBOLayoutBinding.binding = 0;
	mvpUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvpUBOLayoutBinding.descriptorCount = 1;
	mvpUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	mvpUBOLayoutBinding.pImmutableSamplers = NULL; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = NULL; // Optional

	VkDescriptorSetLayoutBinding sceneAttributesUBOLayoutBinding = {};
	sceneAttributesUBOLayoutBinding.binding = 2;
	sceneAttributesUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneAttributesUBOLayoutBinding.descriptorCount = 1;
	sceneAttributesUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	sceneAttributesUBOLayoutBinding.pImmutableSamplers = NULL; // Optional

	VkDescriptorSetLayoutBinding bindings[] = { mvpUBOLayoutBinding,
		samplerLayoutBinding, sceneAttributesUBOLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = sizeof(bindings) / sizeof(VkDescriptorSetLayoutBinding);
	layoutInfo.pBindings = bindings;

	VkDescriptorSetLayout descriptorSetLayout;
	if (vkCreateDescriptorSetLayout(context->device, &layoutInfo, NULL,
		&descriptorSetLayout) != VK_SUCCESS) {

		fprintf(stderr, "Failed to create descriptor set layout.\n");
		return NULL;
	}
	return descriptorSetLayout;
}

static VkVertexInputAttributeDescription getAttributeDescriptions() {

}

static int createGraphicsPipeline(VkContext* context) {

	uint32_t *vertShaderCode, *fragShaderCode;
	long vertShaderLength = readShaderFile("vert.spv", &vertShaderCode);
	long fragShaderLength = readShaderFile("frag.spv", &fragShaderCode);

	context->vertShaderModule = createShaderModule(context->device, vertShaderCode,
		vertShaderLength);
	context->fragShaderModule = createShaderModule(context->device, fragShaderCode,
		fragShaderLength);
	free(vertShaderCode);
	free(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = context->vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = context->fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo,
		fragShaderStageInfo };

	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription attributeDescriptions[5] = {};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(Vertex, pos);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(Vertex, tangent);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[2].offset = offsetof(Vertex, bitangent);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[3].offset = offsetof(Vertex, normal);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[4].offset = offsetof(Vertex, texCoord);

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount =
		sizeof(attributeDescriptions) / sizeof(VkVertexInputAttributeDescription);
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = context->extent.width;
	viewport.height = context->extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = (VkOffset2D) { 0, 0 };
	scissor.extent = context->extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = NULL; /// Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT
		| VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT
		| VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkDescriptorSetLayout setLayouts[] = { context->descriptorSetLayout };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = setLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = 0; // Optional

	if (vkCreatePipelineLayout(context->device, &pipelineLayoutInfo, NULL,
							   &context->pipelineLayout) != VK_SUCCESS) {

		fprintf(stderr, "Failed to create pipeline layout.\n");
		return 0;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = (VkStencilOpState) {}; // Optional
	depthStencil.back = (VkStencilOpState) {}; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = NULL; // Optional
	pipelineInfo.layout = context->pipelineLayout;
	pipelineInfo.renderPass = context->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(context->device, VK_NULL_HANDLE, 1, &pipelineInfo,
		NULL, &context->graphicsPipeline) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create graphics pipeline.\n");
		return 0;
	}
	return 1;
}

static int createFramebuffers(VkContext *context) {

	context->swapChainFramebuffers =
		malloc(context->imageViewCount * sizeof(VkFramebuffer));

	for (uint32_t i = 0; i < context->imageViewCount; ++i) {
		VkImageView attachments[] = {
			context->swapChainImageViews[i],
			context->depthImageView
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = context->renderPass;
		framebufferInfo.attachmentCount = sizeof(attachments) / sizeof(VkImageView);
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = context->extent.width;
		framebufferInfo.height = context->extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(context->device, &framebufferInfo, NULL,
								&context->swapChainFramebuffers[i]) != VK_SUCCESS) {

			fprintf(stderr, "Failed to create framebuffer.\n");
			return 0;
		}
	}
	return 1;
}

static VkCommandPool createCommandPool(const VkContext* const context) {

	int queueFamilyIndex = findQueueFamilies(context->physicalDevice,
		context->surface);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndex;
	poolInfo.flags = 0; // Optional

	VkCommandPool commandPool;
	if (vkCreateCommandPool(context->device, &poolInfo, NULL, &commandPool) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create command pool.\n");
		return NULL;
	}
	return commandPool;
}

static int hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT
		|| format == VK_FORMAT_D24_UNORM_S8_UINT;
}

static uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
	uint32_t typeFilter, VkMemoryPropertyFlags properties) {

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i))
			&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {

			return i;
		}
	}

	fprintf(stderr, "Failed to find suitable memory type.\n");
	return 0;
}

static VkImage createImage(const VkContext* const context, uint32_t width,
						   uint32_t height, uint32_t arrayLayers,
						   VkFormat format, VkImageTiling tiling,
						   VkImageUsageFlags usage,
						   VkMemoryPropertyFlags properties,
						   VkDeviceMemory *imageMemory) {

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = arrayLayers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0; // Optional

	VkImage image;
	if (vkCreateImage(context->device, &imageInfo, NULL, &image) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create texture image.\n");
		return NULL;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(context->device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(context->physicalDevice,
		memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(context->device, &allocInfo, NULL, imageMemory) != VK_SUCCESS) {
		fprintf(stderr, "Failed to allocate texture image memory.\n");
		vkDestroyImage(context->device, image, NULL);
		return NULL;
	}

	vkBindImageMemory(context->device, image, *imageMemory, 0);

	return image;
}

static VkCommandBuffer beginSingleTimeCommands(VkDevice device,
										VkCommandPool commandPool) {

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
		fprintf(stderr, "Could not allocate command buffer.\n");
		return NULL;
	};

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		fprintf(stderr, "Could not begin command buffer.\n");
		return NULL;
	};

	return commandBuffer;
}

static int endSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer,
						   VkCommandPool commandPool, VkQueue graphicsQueue) {

	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		fprintf(stderr, "Could not end command buffer.\n");
		return 0;
	};

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		fprintf(stderr, "Could not submit commands to queue.\n");
		return 0;
	};
	if (vkQueueWaitIdle(graphicsQueue) != VK_SUCCESS) {
		fprintf(stderr, "Could not wait for graphics queue to become idle.\n");
		return 0;
	};

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

static int copyBuffer(VkDevice device, VkCommandPool commandPool,
	VkQueue graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer,
	VkDeviceSize size) {

	VkCommandBuffer commandBuffer;
	VK_CHECK_ERROR(commandBuffer = beginSingleTimeCommands(device, commandPool));

	VkBufferCopy copyRegion = {};
	copyRegion.srcOffset = 0; // Optional
	copyRegion.dstOffset = 0; // Optional
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	VK_CHECK_ERROR(endSingleTimeCommands(device, commandBuffer, commandPool,
		graphicsQueue));
	return 1;
}

static int transitionImageLayout(const VkContext* const context, VkImage image,
								  VkFormat format, VkImageLayout oldLayout,
								  VkImageLayout newLayout, uint32_t layerCount) {

	VkCommandBuffer commandBuffer;
	VK_CHECK_ERROR(commandBuffer = beginSingleTimeCommands(context->device,
		context->commandPool));

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = layerCount;

	if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED
		&& newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {

		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_PREINITIALIZED
			   && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {

		barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			   && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {

		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
			   && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT
			| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	} else {
		fprintf(stderr, "Unsupported layout transition.\n");
		endSingleTimeCommands(context->device, commandBuffer,
			context->commandPool, context->graphicsQueue);
		return 0;
	}

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
		    barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

	VK_CHECK_ERROR(endSingleTimeCommands(context->device, commandBuffer,
		context->commandPool, context->graphicsQueue));
	return 1;
}

static int createDepthResources(VkContext *context) {
	VkFormat depthFormat = findDepthFormat(context);
	if (depthFormat == VK_FORMAT_UNDEFINED) {
		return 0;
	}
	context->depthImage = createImage(context, context->extent.width,
		context->extent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &context->depthImageMemory);
	if (!context->depthImage) {
		return 0;
	}
	context->depthImageView = createImageView(context, context->depthImage,
		depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
	if (!context->depthImageView) {
		return 0;
	}
	if (!transitionImageLayout(context, context->depthImage, depthFormat,
							   VK_IMAGE_LAYOUT_UNDEFINED,
							   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
							   1)) {
		return 0;
	};
	return 1;
}

static int copyBufferToImage(const VkContext* const context, VkBuffer srcBuffer,
							  VkImage dstImage, uint32_t width, uint32_t height,
							  uint32_t layerCount) {

	VkCommandBuffer commandBuffer;
	VK_CHECK_ERROR(commandBuffer = beginSingleTimeCommands(context->device,
		context->commandPool));

	size_t bufferCopyRegionsSize = layerCount * sizeof(VkBufferImageCopy);
	VkBufferImageCopy *bufferCopyRegions = malloc(bufferCopyRegionsSize);
	memset(bufferCopyRegions, 0, bufferCopyRegionsSize);

	VkDeviceSize offset = 0;
	for (uint32_t i = 0; i < layerCount; ++i) {
		bufferCopyRegions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegions[i].imageSubresource.mipLevel = 0;
		bufferCopyRegions[i].imageSubresource.baseArrayLayer = i;
		bufferCopyRegions[i].imageSubresource.layerCount = 1;
		bufferCopyRegions[i].imageExtent.width = width;
		bufferCopyRegions[i].imageExtent.height = height;
		bufferCopyRegions[i].imageExtent.depth = 1;
		bufferCopyRegions[i].bufferOffset = offset;

		offset += width * height * 4;
	}

	vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount, bufferCopyRegions);

	free(bufferCopyRegions);

	VK_CHECK_ERROR(endSingleTimeCommands(context->device, commandBuffer,
		context->commandPool, context->graphicsQueue));
	return 1;
}

static VkBuffer createBuffer(const VkContext* const context,
	VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties, VkDeviceMemory *bufferMemory) {

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer buffer;
	if (vkCreateBuffer(context->device, &bufferInfo, NULL, &buffer) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create vertex buffer.\n");
		return NULL;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(context->device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(context->physicalDevice,
		memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(context->device, &allocInfo, NULL, bufferMemory) != VK_SUCCESS) {
		fprintf(stderr, "Failed to allocate vertex buffer memory.\n");
		vkDestroyBuffer(context->device, buffer, NULL);
		return NULL;
	}

	vkBindBufferMemory(context->device, buffer, *bufferMemory, 0);

	return buffer;
}

static VkDeviceSize readTextureFile(const char* const filename, TexHdr *textureHeader,
							uint8_t **pixels) {

	size_t numSearchPaths = sizeof(TEXTURE_SEARCH_PATHS) / sizeof(char*);
	FILE *file = openFile(filename, TEXTURE_SEARCH_PATHS, numSearchPaths);

	fread(textureHeader, sizeof(TexHdr), 1, file);
	if (textureHeader->type != 0x1401) { // GL_UNSIGNED_BYTE
		fprintf(stderr, "Texture has unsupported type: %u\n",
			textureHeader->type);
		fclose(file);
		return 0;
	}
	VkDeviceSize dataSize = textureHeader->width * textureHeader->height;
	VkDeviceSize pixelSize;
	switch(textureHeader->format) {
		case 0x1907: // GL_RGB
			pixelSize = 3;
			break;
		case 0x1908: // GL_RGBA
			pixelSize = 4;
			break;
		default:
			fprintf(stderr, "Texture has unsupported format: %u\n",
				textureHeader->format);
			fclose(file);
			return 0;
	}
	dataSize *= pixelSize;
	*pixels = malloc(dataSize);
	fread(*pixels, sizeof(uint8_t), dataSize, file);
	fclose(file);

	// If the format is RGB, convert it to RGBA
	if (pixelSize == 3) {
		uint8_t *newPixels = malloc(dataSize / 3 * 4);
		uint8_t *ptr = newPixels;
		for (size_t i = 0; i < dataSize; i += 3) {
			memcpy(ptr, &(*pixels)[i], 3);
			ptr[3] = 255; // Alpha
			ptr += 4;
		}
		free(*pixels);
		*pixels = newPixels;
		dataSize /= 3;
		dataSize *= 4;
	}

	return dataSize;
}

static int createTextureImage(VkContext *context) {
	TexHdr diffuseTextureHeader, normalTextureHeader;
	uint8_t *diffusePixels, *normalPixels;
	VkDeviceSize diffuseDataSize = readTextureFile("brick.tex",
		&diffuseTextureHeader, &diffusePixels);
	VkDeviceSize normalDataSize = readTextureFile("normal.tex",
		&normalTextureHeader, &normalPixels);

	if (memcmp(&diffuseTextureHeader, &normalTextureHeader, sizeof(TexHdr))
		|| diffuseDataSize != normalDataSize) {

		fprintf(stderr, "Diffuse and normal textures are incompatible.\n");
		free(diffusePixels);
		free(normalPixels);
		return 0;
	}

	VkDeviceMemory stagingBufferMemory;
	VkBuffer stagingBuffer = createBuffer(context,
		diffuseDataSize + normalDataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory);
	if (stagingBuffer == NULL) {
		fprintf(stderr, "Could not create staging buffer.\n");
		free(diffusePixels);
		free(normalPixels);
		return 0;
	}

	void* data;
	vkMapMemory(context->device, stagingBufferMemory, 0,
		diffuseDataSize + normalDataSize, 0, &data);
	memcpy(data, diffusePixels, diffuseDataSize);
	memcpy(data + diffuseDataSize, normalPixels, normalDataSize);
	vkUnmapMemory(context->device, stagingBufferMemory);
	free(diffusePixels);
	free(normalPixels);

	VK_CHECK_ERROR(context->textureImage = createImage(context,
		diffuseTextureHeader.width, diffuseTextureHeader.height, 2,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    	VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &context->textureImageMemory));

	VK_CHECK_ERROR(transitionImageLayout(context, context->textureImage,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 2));
	VK_CHECK_ERROR(copyBufferToImage(context, stagingBuffer, context->textureImage,
		diffuseTextureHeader.width, diffuseTextureHeader.height, 2));
	VK_CHECK_ERROR(transitionImageLayout(context, context->textureImage,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 2));

	vkFreeMemory(context->device, stagingBufferMemory, NULL);
	vkDestroyBuffer(context->device, stagingBuffer, NULL);
	return 1;
}

static VkImageView createTextureImageView(const VkContext* const context) {
	return createImageView(context, context->textureImage,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 2);
}

static VkSampler createTextureSampler(const VkContext* const context) {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler textureSampler;
	if (vkCreateSampler(context->device, &samplerInfo, NULL, &textureSampler) != VK_SUCCESS) {
    	fprintf(stderr, "Failed to create texture sampler.\n");
		return NULL;
    }
	return textureSampler;
}

static int createVertexBuffer(VkContext *context) {
	VkDeviceSize bufferSize = sizeof(CUBE_VERTICES);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VK_CHECK_ERROR(stagingBuffer = createBuffer(context, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory));

	void* data;
	vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, CUBE_VERTICES, bufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	VK_CHECK_ERROR(context->vertexBuffer = createBuffer(context, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &context->vertexBufferMemory));

	VK_CHECK_ERROR(copyBuffer(context->device, context->commandPool,
		context->graphicsQueue, stagingBuffer, context->vertexBuffer,
		bufferSize));

	vkFreeMemory(context->device, stagingBufferMemory, NULL);
	vkDestroyBuffer(context->device, stagingBuffer, NULL);
	return 1;
}

static int createIndexBuffer(VkContext *context) {
	VkDeviceSize bufferSize = sizeof(CUBE_INDICES);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VK_CHECK_ERROR(stagingBuffer = createBuffer(context, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBufferMemory));

	void* data;
	vkMapMemory(context->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, CUBE_INDICES, bufferSize);
	vkUnmapMemory(context->device, stagingBufferMemory);

	VK_CHECK_ERROR(context->indexBuffer = createBuffer(context, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &context->indexBufferMemory));

	VK_CHECK_ERROR(copyBuffer(context->device, context->commandPool,
		context->graphicsQueue, stagingBuffer, context->indexBuffer, bufferSize));

	vkFreeMemory(context->device, stagingBufferMemory, NULL);
	vkDestroyBuffer(context->device, stagingBuffer, NULL);
	return 1;
}

static int createMVPUniformBuffer(VkContext *context) {
	VkDeviceSize bufferSize = sizeof(MVPMatrices);

	VK_CHECK_ERROR(context->mvpUniformBuffer = createBuffer(context, bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&context->mvpUniformBufferMemory));
	return 1;
}

static int createSceneAttributesUniformBuffer(VkContext *context) {
	VkDeviceSize bufferSize = sizeof(SceneAttributes);

	VK_CHECK_ERROR(context->sceneAttributesUniformBuffer = createBuffer(context,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		&context->sceneAttributesUniformBufferMemory));
	return 1;
}

static VkDescriptorPool createDescriptorPool(const VkContext* const context) {
	VkDescriptorPoolSize poolSizes[2] = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(VkDescriptorPoolSize);
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = 1;

	VkDescriptorPool descriptorPool;
	if (vkCreateDescriptorPool(context->device, &poolInfo, NULL,
							   &descriptorPool) != VK_SUCCESS) {

		fprintf(stderr, "Failed to create descriptor pool.\n");
		return NULL;
	}
	return descriptorPool;
}

static VkDescriptorSet createDescriptorSet(const VkContext* const context) {

	VkDescriptorSetLayout layouts[] = { context->descriptorSetLayout };
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = context->descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = layouts;

	VkDescriptorSet descriptorSet;
	if (vkAllocateDescriptorSets(context->device, &allocInfo,
								 &descriptorSet) != VK_SUCCESS) {

		fprintf(stderr, "Failed to allocate descriptor set.\n");
		return NULL;
	}

	VkDescriptorBufferInfo mvpBufferInfo = {};
	mvpBufferInfo.buffer = context->mvpUniformBuffer;
	mvpBufferInfo.offset = 0;
	mvpBufferInfo.range = sizeof(MVPMatrices);

	VkDescriptorBufferInfo sceneAttributesBufferInfo = {};
	sceneAttributesBufferInfo.buffer = context->sceneAttributesUniformBuffer;
	sceneAttributesBufferInfo.offset = 0;
	sceneAttributesBufferInfo.range = sizeof(SceneAttributes);

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = context->textureImageView;
	imageInfo.sampler = context->textureSampler;

	VkWriteDescriptorSet descriptorWrites[3] = {};
	descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet = descriptorSet;
	descriptorWrites[0].dstBinding = 0;
	descriptorWrites[0].dstArrayElement = 0;
	descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount = 1;
	descriptorWrites[0].pBufferInfo = &mvpBufferInfo;

	descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet = descriptorSet;
	descriptorWrites[1].dstBinding = 1;
	descriptorWrites[1].dstArrayElement = 0;
	descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount = 1;
	descriptorWrites[1].pImageInfo = &imageInfo;

	descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[2].dstSet = descriptorSet;
	descriptorWrites[2].dstBinding = 2;
	descriptorWrites[2].dstArrayElement = 0;
	descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[2].descriptorCount = 1;
	descriptorWrites[2].pBufferInfo = &sceneAttributesBufferInfo;

	uint32_t descriptorWriteCount =
		sizeof(descriptorWrites) / sizeof(VkWriteDescriptorSet);
	vkUpdateDescriptorSets(context->device, descriptorWriteCount,
		descriptorWrites, 0, NULL);

	return descriptorSet;
}

static int createCommandBuffers(VkContext *context) {

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = context->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = context->imageViewCount;

	context->commandBuffers = malloc(context->imageViewCount * sizeof(VkCommandBuffer));
	if (vkAllocateCommandBuffers(context->device, &allocInfo,
								 context->commandBuffers) != VK_SUCCESS) {

		fprintf(stderr, "Failed to allocate command buffers.\n");
		free(context->commandBuffers);
		context->commandBuffers = NULL;
		return 0;
	}

	for (uint32_t i = 0; i < context->imageViewCount; i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = NULL; // Optional

		vkBeginCommandBuffer(context->commandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = context->renderPass;
		renderPassInfo.framebuffer = context->swapChainFramebuffers[i];
		renderPassInfo.renderArea.offset = (VkOffset2D) { 0, 0 };
		renderPassInfo.renderArea.extent = context->extent;

		VkClearValue clearValues[] = { {}, {} };
		clearValues[0].color = (VkClearColorValue) { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = (VkClearDepthStencilValue) { 1.0f, 0 };

		renderPassInfo.clearValueCount = sizeof(clearValues) / sizeof(VkClearValue);
		renderPassInfo.pClearValues = clearValues;

		vkCmdBeginRenderPass(context->commandBuffers[i], &renderPassInfo,
			VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(context->commandBuffers[i],
			VK_PIPELINE_BIND_POINT_GRAPHICS, context->graphicsPipeline);

		VkBuffer vertexBuffers[] = { context->vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(context->commandBuffers[i], 0, 1, vertexBuffers,
			offsets);
		vkCmdBindIndexBuffer(context->commandBuffers[i], context->indexBuffer,
			0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(context->commandBuffers[i],
			VK_PIPELINE_BIND_POINT_GRAPHICS, context->pipelineLayout, 0, 1,
			&context->descriptorSet, 0, NULL);
		vkCmdDrawIndexed(context->commandBuffers[i],
			sizeof(CUBE_INDICES) / sizeof(uint16_t), 1, 0, 0, 0);

		vkCmdEndRenderPass(context->commandBuffers[i]);
		if (vkEndCommandBuffer(context->commandBuffers[i]) != VK_SUCCESS) {
			fprintf(stderr, "Failed to record command buffer.\n");
			return 0;
		}
	}
	return 1;
}

static int createSemaphores(VkContext *context) {
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	if (vkCreateSemaphore(context->device, &semaphoreInfo, NULL,
						  &context->imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(context->device, &semaphoreInfo, NULL,
						  &context->renderFinishedSemaphore) != VK_SUCCESS) {

		fprintf(stderr, "Failed to create semaphores.\n");
		return 0;
	}
	return 1;
}

int initVulkan(GLFWwindow *window, VkContext *context, int vsync) {
	VK_CHECK_ERROR(context->instance = createInstance());
	VK_CHECK_ERROR(context->surface = createSurface(context->instance, window));
	VK_CHECK_ERROR(context->physicalDevice = pickPhysicalDevice(context));

	int queueFamilyIndex;
	VK_CHECK_ERROR(context->device = createDevice(context, &queueFamilyIndex));

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	VK_CHECK_ERROR(createSwapChain(context, width, height, vsync));

	VK_CHECK_ERROR(createImageViews(context));
	VK_CHECK_ERROR(context->renderPass = createRenderPass(context));
	VK_CHECK_ERROR(context->descriptorSetLayout = createDescriptorSetLayout(context));
	VK_CHECK_ERROR(createGraphicsPipeline(context));
	VK_CHECK_ERROR(context->commandPool = createCommandPool(context));

	vkGetDeviceQueue(context->device, queueFamilyIndex, 0,
		&context->presentQueue);
	vkGetDeviceQueue(context->device, queueFamilyIndex, 0,
		&context->graphicsQueue);

	VK_CHECK_ERROR(createDepthResources(context));
	VK_CHECK_ERROR(createFramebuffers(context));

	VK_CHECK_ERROR(createTextureImage(context));
	VK_CHECK_ERROR(context->textureImageView = createTextureImageView(context));
	VK_CHECK_ERROR(context->textureSampler = createTextureSampler(context));

	VK_CHECK_ERROR(createVertexBuffer(context));
	VK_CHECK_ERROR(createIndexBuffer(context));
	VK_CHECK_ERROR(createMVPUniformBuffer(context));
	VK_CHECK_ERROR(createSceneAttributesUniformBuffer(context));

	VK_CHECK_ERROR(context->descriptorPool = createDescriptorPool(context));
	VK_CHECK_ERROR(context->descriptorSet = createDescriptorSet(context));

	VK_CHECK_ERROR(createCommandBuffers(context));
	VK_CHECK_ERROR(createSemaphores(context));

	return 1;
}

void destroyVulkan(const VkContext* const context) {
	if (context->device) {
		vkDeviceWaitIdle(context->device);
	}

	VK_DESTROY(context->device, context->imageAvailableSemaphore,
		vkDestroySemaphore);
	VK_DESTROY(context->device, context->renderFinishedSemaphore,
		vkDestroySemaphore);

	if (context->device && context->commandPool && context->imageViewCount
		&& context->commandBuffers) {

		vkFreeCommandBuffers(context->device, context->commandPool,
			context->imageViewCount, context->commandBuffers);
	}
	free(context->commandBuffers);

	VK_DESTROY(context->device, context->descriptorPool, vkDestroyDescriptorPool);

	VK_DESTROY(context->device, context->sceneAttributesUniformBufferMemory, vkFreeMemory);
	VK_DESTROY(context->device, context->sceneAttributesUniformBuffer, vkDestroyBuffer);

	VK_DESTROY(context->device, context->mvpUniformBufferMemory, vkFreeMemory);
	VK_DESTROY(context->device, context->mvpUniformBuffer, vkDestroyBuffer);

	VK_DESTROY(context->device, context->indexBufferMemory, vkFreeMemory);
	VK_DESTROY(context->device, context->indexBuffer, vkDestroyBuffer);

	VK_DESTROY(context->device, context->vertexBufferMemory, vkFreeMemory);
	VK_DESTROY(context->device, context->vertexBuffer, vkDestroyBuffer);

	VK_DESTROY(context->device, context->textureSampler, vkDestroySampler);

	VK_DESTROY(context->device, context->textureImageView, vkDestroyImageView);
	VK_DESTROY(context->device, context->textureImageMemory, vkFreeMemory);
	VK_DESTROY(context->device, context->textureImage, vkDestroyImage);

	VK_DESTROY(context->device, context->depthImageView, vkDestroyImageView);
	VK_DESTROY(context->device, context->depthImageMemory, vkFreeMemory);
	VK_DESTROY(context->device, context->depthImage, vkDestroyImage);

	VK_DESTROY(context->device, context->commandPool, vkDestroyCommandPool);

	for (uint32_t i = 0; i < context->imageViewCount; ++i) {
		VK_DESTROY(context->device, context->swapChainFramebuffers[i],
			vkDestroyFramebuffer);
	}
	free(context->swapChainFramebuffers);

	VK_DESTROY(context->device, context->graphicsPipeline, vkDestroyPipeline);
	VK_DESTROY(context->device, context->pipelineLayout, vkDestroyPipelineLayout);

	VK_DESTROY(context->device, context->descriptorSetLayout,
		vkDestroyDescriptorSetLayout);

	VK_DESTROY(context->device, context->renderPass, vkDestroyRenderPass);

	VK_DESTROY(context->device, context->vertShaderModule, vkDestroyShaderModule);
	VK_DESTROY(context->device, context->fragShaderModule, vkDestroyShaderModule);

	for (uint32_t i = 0; i < context->imageViewCount; ++i) {
		VK_DESTROY(context->device, context->swapChainImageViews[i], vkDestroyImageView);
	}
	free(context->swapChainImageViews);

	VK_DESTROY(context->device, context->swapChain, vkDestroySwapchainKHR);

	VK_DESTROY(context->instance, context->surface, vkDestroySurfaceKHR);

	if (context->device) {
		vkDestroyDevice(context->device, NULL);
	}

	if (context->instance) {
		vkDestroyInstance(context->instance, NULL);
	}
}

