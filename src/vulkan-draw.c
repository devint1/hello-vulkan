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

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <GLFW/glfw3.h>

#include "glfw-controls.h"
#include "maths.h"
#include "scene.h"

void drawFrame(const VkContext* const context) {
	uint32_t imageIndex;
	vkAcquireNextImageKHR(context->device, context->swapChain, ULLONG_MAX,
		context->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { context->imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] =
		{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &context->commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { context->renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(context->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		fprintf(stderr, "Failed to submit draw command buffer.\n");
		return;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { context->swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = NULL; // Optional

	vkQueuePresentKHR(context->presentQueue, &presentInfo);

	vkQueueWaitIdle(context->presentQueue);
}

UBOAttributes initializeUBOAttributes(float width, float height) {
	UBOAttributes uboAttributes;

	identityMatrix(uboAttributes.mvp.model);
	identityMatrix(uboAttributes.mvp.proj);

	memcpy(uboAttributes.sceneAttributes.eyePos, EYE, sizeof(EYE));
	uboAttributes.yaw = 90.0f;
	uboAttributes.pitch = 0.0f;
	eulerView(uboAttributes.mvp.view, uboAttributes.sceneAttributes.eyePos,
		uboAttributes.pitch, uboAttributes.yaw);

	perspectiveMatrix(uboAttributes.mvp.proj, 45.0f, width / height, 0.1f,
		100000.0f);

	memcpy(uboAttributes.sceneAttributes.ambientColor, CUBE_AMBIENT,
		sizeof(CUBE_AMBIENT));
	memcpy(uboAttributes.sceneAttributes.diffuseColor, CUBE_DIFFUSE,
		sizeof(CUBE_AMBIENT));
	memcpy(uboAttributes.sceneAttributes.specularColor, CUBE_SPECULAR,
		sizeof(CUBE_AMBIENT));
	memcpy(uboAttributes.sceneAttributes.eyePos, EYE, sizeof(EYE));
	memcpy(uboAttributes.sceneAttributes.lightPos, LIGHT_POS,
		sizeof(LIGHT_POS));
	memcpy(uboAttributes.sceneAttributes.lightColor, LIGHT_COLOR,
		sizeof(LIGHT_COLOR));
	uboAttributes.sceneAttributes.specularExp = CUBE_SPECULAR_EXP;

	return uboAttributes;
}

void updateUniformBuffer(GLFWwindow *window, UBOAttributes *uboAttributes,
						 const VkContext* const context) {
	applyUBOControls(window, uboAttributes);

	void *data;
	vkMapMemory(context->device, context->mvpUniformBufferMemory, 0,
		sizeof(MVPMatrices), 0, &data);
	memcpy(data, &uboAttributes->mvp, sizeof(MVPMatrices));
	vkUnmapMemory(context->device, context->mvpUniformBufferMemory);

	vkMapMemory(context->device, context->sceneAttributesUniformBufferMemory, 0,
		sizeof(SceneAttributes), 0, &data);
	memcpy(data, &uboAttributes->sceneAttributes, sizeof(SceneAttributes));
	vkUnmapMemory(context->device, context->sceneAttributesUniformBufferMemory);
}

