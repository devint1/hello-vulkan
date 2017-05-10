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

#pragma once

#include <vulkan/vulkan.h>

typedef struct _VkContext {
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkSurfaceFormatKHR surfaceFormat;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapChain;
	uint32_t imageViewCount;
	VkImageView *swapChainImageViews;
	VkShaderModule vertShaderModule, fragShaderModule;
	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipeline graphicsPipeline;
	VkFramebuffer *swapChainFramebuffers;
	VkCommandPool commandPool;
	VkCommandBuffer *commandBuffers;
	VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;
	VkQueue presentQueue, graphicsQueue;
	VkBuffer vertexBuffer, indexBuffer, mvpUniformBuffer,
		sceneAttributesUniformBuffer;
	VkDeviceMemory vertexBufferMemory, indexBufferMemory,
		mvpUniformBufferMemory, sceneAttributesUniformBufferMemory,
		textureImageMemory, depthImageMemory;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
	VkImage textureImage, depthImage;
	VkImageView textureImageView, depthImageView;
	VkSampler textureSampler;
	VkExtent2D extent;
} VkContext;

typedef struct _MVPMatrices {
	float model[16], view[16], proj[16];
} MVPMatrices;

typedef struct _SceneAttributes {
	float ambientColor[4], diffuseColor[4], specularColor[4], eyePos[4],
		lightPos[4], lightColor[4], specularExp;
} SceneAttributes;

typedef struct _UBOAttributes {
	MVPMatrices mvp;
	SceneAttributes sceneAttributes;
	float pitch, yaw;
	double lastCursorX, lastCursorY;
} UBOAttributes;

typedef struct _Vertex {
	const float pos[3], tangent[3], bitangent[3], normal[3], texCoord[2];
} Vertex;

