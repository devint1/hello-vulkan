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

#include "glfw-controls.h"
#include "maths.h"

#define CONTROL_CAMERA_FORWARD	1
#define CONTROL_CAMERA_REVERSE	2
#define CONTROL_CAMERA_LEFT		4
#define CONTROL_CAMERA_RIGHT	8
#define CONTROL_CAMERA_UP		16
#define CONTROL_CAMERA_DOWN		32
#define CONTROL_LIGHT_FORWARD	64
#define CONTROL_LIGHT_REVERSE	128
#define CONTROL_LIGHT_LEFT		256
#define CONTROL_LIGHT_RIGHT		512
#define CONTROL_LIGHT_UP		1024
#define CONTROL_LIGHT_DOWN		2048
#define CONTROL_ROTATE_CAMERA	4096
#define CONTROL_ROTATE_MODEL	8192
#define CONTROL_CLOSE			16384

#define MOVE_SPEED 0.05f
#define MOUSE_SENSITIVITY 0.1f

static uint16_t getKeyboardControlsState(GLFWwindow* window) {
	uint16_t controlsMask = 0;
	int keyState;

	controlsMask |= glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS
		? CONTROL_CAMERA_FORWARD : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS
		? CONTROL_CAMERA_REVERSE : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS
		? CONTROL_CAMERA_LEFT : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS
		? CONTROL_CAMERA_RIGHT : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS
		? CONTROL_CAMERA_UP : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS
		? CONTROL_CAMERA_DOWN : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS
		? CONTROL_LIGHT_FORWARD : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS
		? CONTROL_LIGHT_REVERSE : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS
		? CONTROL_LIGHT_LEFT : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS
		? CONTROL_LIGHT_RIGHT : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS
		? CONTROL_LIGHT_UP : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS
		? CONTROL_LIGHT_DOWN : 0;
	controlsMask |= glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
		? CONTROL_CLOSE : 0;

	return controlsMask;
}

static void getMouseControlsState(GLFWwindow *window, double *x, double *y,
								  uint16_t *controlsMask) {

	*controlsMask |= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)
		== GLFW_PRESS ? CONTROL_ROTATE_CAMERA : 0;
	*controlsMask |= glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)
		== GLFW_PRESS ? CONTROL_ROTATE_MODEL : 0;

	glfwGetCursorPos(window, x, y);
}

void applyUBOControls(GLFWwindow *window, UBOAttributes *uboAttributes) {
	uint16_t controlsMask;
	double cursorX, cursorY, deltaX, deltaY;

	controlsMask = getKeyboardControlsState(window);
	if (controlsMask & CONTROL_CLOSE) {
		glfwSetWindowShouldClose(window, 1);
		return;
	}
	getMouseControlsState(window, &cursorX, &cursorY, &controlsMask);
	deltaX = (cursorX - uboAttributes->lastCursorX) * MOUSE_SENSITIVITY;
	deltaY = (cursorY - uboAttributes->lastCursorY) * MOUSE_SENSITIVITY;
	if (controlsMask & CONTROL_ROTATE_CAMERA) {
		uboAttributes->yaw -= deltaX;
		uboAttributes->pitch += deltaY;
	}
	if (controlsMask & CONTROL_ROTATE_MODEL) {
		rotateMatrix(uboAttributes->mvp.model, -deltaY * 2.0f, 0.0f, 0.0f, 1.0f);
		rotateMatrix(uboAttributes->mvp.model, deltaX * 2.0f, 0.0f, 1.0f, 0.0f);
	}
	glfwGetCursorPos(window, &uboAttributes->lastCursorX,
		&uboAttributes->lastCursorY);

	if (controlsMask & CONTROL_CAMERA_FORWARD) {
		uboAttributes->sceneAttributes.eyePos[0] -= MOVE_SPEED * uboAttributes->mvp.view[2];
		uboAttributes->sceneAttributes.eyePos[1] -= MOVE_SPEED * uboAttributes->mvp.view[6];
		uboAttributes->sceneAttributes.eyePos[2] -= MOVE_SPEED * uboAttributes->mvp.view[10];
	}
	if (controlsMask & CONTROL_CAMERA_REVERSE) {
		uboAttributes->sceneAttributes.eyePos[0] += MOVE_SPEED * uboAttributes->mvp.view[2];
		uboAttributes->sceneAttributes.eyePos[1] += MOVE_SPEED * uboAttributes->mvp.view[6];
		uboAttributes->sceneAttributes.eyePos[2] += MOVE_SPEED * uboAttributes->mvp.view[10];
	}
	if (controlsMask & CONTROL_CAMERA_LEFT) {
		uboAttributes->sceneAttributes.eyePos[0] -= MOVE_SPEED * uboAttributes->mvp.view[0];
		uboAttributes->sceneAttributes.eyePos[1] -= MOVE_SPEED * uboAttributes->mvp.view[4];
		uboAttributes->sceneAttributes.eyePos[2] -= MOVE_SPEED * uboAttributes->mvp.view[8];
	}
	if (controlsMask & CONTROL_CAMERA_RIGHT) {
		uboAttributes->sceneAttributes.eyePos[0] += MOVE_SPEED * uboAttributes->mvp.view[0];
		uboAttributes->sceneAttributes.eyePos[1] += MOVE_SPEED * uboAttributes->mvp.view[4];
		uboAttributes->sceneAttributes.eyePos[2] += MOVE_SPEED * uboAttributes->mvp.view[8];
	}
	if (controlsMask & CONTROL_CAMERA_UP) {
		uboAttributes->sceneAttributes.eyePos[0] += MOVE_SPEED * uboAttributes->mvp.view[1];
		uboAttributes->sceneAttributes.eyePos[1] += MOVE_SPEED * uboAttributes->mvp.view[5];
		uboAttributes->sceneAttributes.eyePos[2] += MOVE_SPEED * uboAttributes->mvp.view[9];
	}
	if (controlsMask & CONTROL_CAMERA_DOWN) {
		uboAttributes->sceneAttributes.eyePos[0] -= MOVE_SPEED * uboAttributes->mvp.view[1];
		uboAttributes->sceneAttributes.eyePos[1] -= MOVE_SPEED * uboAttributes->mvp.view[5];
		uboAttributes->sceneAttributes.eyePos[2] -= MOVE_SPEED * uboAttributes->mvp.view[9];
	}
	if (controlsMask & CONTROL_LIGHT_FORWARD) {
		uboAttributes->sceneAttributes.lightPos[0] -= MOVE_SPEED;
	}
	if (controlsMask & CONTROL_LIGHT_REVERSE) {
		uboAttributes->sceneAttributes.lightPos[0] += MOVE_SPEED;
	}
	if (controlsMask & CONTROL_LIGHT_LEFT) {
		uboAttributes->sceneAttributes.lightPos[2] -= MOVE_SPEED;
	}
	if (controlsMask & CONTROL_LIGHT_RIGHT) {
		uboAttributes->sceneAttributes.lightPos[2] += MOVE_SPEED;
	}
	if (controlsMask & CONTROL_LIGHT_UP) {
		uboAttributes->sceneAttributes.lightPos[1] -= MOVE_SPEED;
	}
	if (controlsMask & CONTROL_LIGHT_DOWN) {
		uboAttributes->sceneAttributes.lightPos[1] += MOVE_SPEED;
	}
	eulerView(uboAttributes->mvp.view, uboAttributes->sceneAttributes.eyePos,
		uboAttributes->pitch, uboAttributes->yaw);
}

