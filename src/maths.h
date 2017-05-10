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

void multMatrix(float *result, const float* const srcA,
	const float* const srcB);

void translateMatrix(float *result, float x, float y, float z);

void perspectiveMatrix(float *result, float fov, float aspect, float nearZ,
	float farZ);

void identityMatrix(float *result);

void rotateMatrix(float *result, float angle, float x, float y,
	float z);

void scaleMatrix(float *result, float sx, float sy, float sz);

void lookAt(float *result, const float* const eye,
	const float* const center, const float* const up);

void eulerView(float *result, const float* const eye, float pitch, float yaw);

#define MAX(a,b) ((a) > (b) ? a : b)
#define MIN(a,b) ((a) < (b) ? a : b)

