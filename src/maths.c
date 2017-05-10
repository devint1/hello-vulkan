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

#include <math.h>
#include <stdlib.h>
#include <string.h>

static float radians(float degrees) {
	return degrees * M_PI / 180.0f;
}

void multMatrix(float *result, const float* const srcA,
	const float* const srcB) {

	int i, j, k;
	float tmp[16];

	memset(tmp, 0, sizeof(tmp));
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			for (k = 0; k < 4; ++k) {
				tmp[i * 4 + j] += srcA[i * 4 + k] * srcB[k * 4 + j];
			}
		}
	}
	memcpy(result, tmp, sizeof(tmp));
}

void translateMatrix(float *result, float x, float y, float z) {
	float trans[16];

	memset(trans, 0, sizeof(trans));
	trans[0] = 1.0f;
	trans[12] = result[0] * x + result[4] * y + result[8] * z + result[12];
	trans[5] = 1.0f;
	trans[13] = result[1] * x + result[5] * y + result[9] * z + result[13];
	trans[10] = 1.0f;
	trans[14] = result[2] * x + result[6] * y + result[10] * z + result[14];
	trans[15] = result[3] * x + result[7] * y + result[11] * z + result[15];

	multMatrix(result, result, trans);
}

void perspectiveMatrix(float *result, float fov, float aspect, float nearZ,
	float farZ) {

	float left, right, top, bottom;
	top = tanf(radians(fov) / 2.0f) * nearZ;
	right = top * aspect;
	bottom = -top;
	left = bottom * aspect;

	float deltaX = right - left;
	float deltaY = top - bottom;
	float deltaZ = farZ - nearZ;
	float frust[16];

	if ((nearZ <= 0.0f) || (farZ <= 0.0f) ||
		(deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f)) {
		return;
	}

	frust[0] = 2.0f * nearZ / deltaX;
	frust[1] = 0.0f;
	frust[2] = 0.0f;
	frust[3] = 0.0f;

	frust[4] = 0.0f;
	frust[5] = -2.0f * nearZ / deltaY;
	frust[6] = 0.0f;
	frust[7] = 0.0f;

	frust[8] = (right + left) / deltaX;
	frust[9] = (top + bottom) / deltaY;
	frust[10] = -(nearZ + farZ) / deltaZ;
	frust[11] = -1.0f;

	frust[12] = 0.0f;
	frust[13] = 0.0f;
	frust[14] = -2.0f * nearZ * farZ / deltaZ;
	frust[15] = 0.0f;

	multMatrix(result, result, frust);
}

void identityMatrix(float *result) {
	result[0] = 1.0f;
	result[1] = 0.0f;
	result[2] = 0.0f;
	result[3] = 0.0f;

	result[4] = 0.0f;
	result[5] = 1.0f;
	result[6] = 0.0f;
	result[7] = 0.0f;

	result[8] = 0.0f;
	result[9] = 0.0f;
	result[10] = 1.0f;
	result[11] = 0.0f;

	result[12] = 0.0f;
	result[13] = 0.0f;
	result[14] = 0.0f;
	result[15] = 1.0f;
}

void rotateMatrix(float *result, float angle, float x, float y, float z) {
	float sinAngle, cosAngle;
	float mag = sqrtf(x * x + y * y + z * z);

	if (mag <= 0.0f) {
		return;
	}

	sinAngle = sinf(radians(angle));
	cosAngle = cosf(radians(angle));
	float xx, yy, zz, xy, yz, zx, xs, ys, zs;
	float oneMinusCos;
	float rotMat[16];

	x /= mag;
	y /= mag;
	z /= mag;

	xx = x * x;
	yy = y * y;
	zz = z * z;
	xy = x * y;
	yz = y * z;
	zx = z * x;
	xs = x * sinAngle;
	ys = y * sinAngle;
	zs = z * sinAngle;
	oneMinusCos = 1.0f - cosAngle;

	rotMat[0] = (oneMinusCos * xx) + cosAngle;
	rotMat[1] = (oneMinusCos * xy) - zs;
	rotMat[2] = (oneMinusCos * zx) + ys;
	rotMat[3] = 0.0f;

	rotMat[4] = (oneMinusCos * xy) + zs;
	rotMat[5] = (oneMinusCos * yy) + cosAngle;
	rotMat[6] = (oneMinusCos * yz) - xs;
	rotMat[7] = 0.0f;

	rotMat[8] = (oneMinusCos * zx) - ys;
	rotMat[9] = (oneMinusCos * yz) + xs;
	rotMat[10] = (oneMinusCos * zz) + cosAngle;
	rotMat[11] = 0.0f;

	rotMat[12] = 0.0f;
	rotMat[13] = 0.0f;
	rotMat[14] = 0.0f;
	rotMat[15] = 1.0f;

	multMatrix(result, result, rotMat);
}

void scaleMatrix(float *result, float sx, float sy, float sz) {
	result[0] *= sx;
	result[1] *= sx;
	result[2] *= sx;
	result[3] *= sx;

	result[4] *= sy;
	result[5] *= sy;
	result[6] *= sy;
	result[7] *= sy;

	result[8] *= sz;
	result[9] *= sz;
	result[10] *= sz;
	result[11] *= sz;
}

static float dot(const float* const v1, const float* const v2) {
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

static void cross(float *result, float *v1, float *v2) {
	float a2b3, a3b2, a3b1, a1b3, a1b2, a2b1;

	a2b3 = v1[1] * v2[2];
	a3b2 = v1[2] * v2[1];
	a3b1 = v1[2] * v2[0];
	a1b3 = v1[0] * v2[2];
	a1b2 = v1[0] * v2[1];
	a2b1 = v1[1] * v2[0];

	result[0] = a2b3 - a3b2;
	result[1] = a3b1 - a1b3;
	result[2] = a1b2 - a2b1;
}

static void normalize(float *v) {
	float mag = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= mag;
	v[1] /= mag;
	v[2] /= mag;
}

void lookAt(float *result, const float* const eye,
	const float* const center, const float* const up) {

	float forward[3], side[3], newUp[3];
	float look[16];

	forward[0] = center[0] - eye[0];
	forward[1] = center[1] - eye[1];
	forward[2] = center[2] - eye[2];
	normalize(forward);
	memcpy(newUp, up, sizeof(float) * 3);
	cross(side, forward, newUp);
	normalize(side);
	cross(newUp, side, forward);

	result[0] = side[0];
    result[4] = side[1];
    result[8] = side[2];
    result[12] = 0.0f;

    result[1] = newUp[0];
    result[5] = newUp[1];
    result[9] = newUp[2];
    result[13] = 0.0f;

    result[2] = -forward[0];
    result[6] = -forward[1];
    result[10] = -forward[2];
    result[14] = 0.0f;

    result[3] = result[7] = result[11] = 0.0f;
    result[15] = 1.0f;

    translateMatrix(result, -eye[0], -eye[1], -eye[2]);
}

void eulerView(float *result, const float* const eye, float pitch, float yaw) {
	pitch = radians(pitch);
	yaw = radians(yaw);

	float cosPitch = cosf(pitch);
	float sinPitch = sinf(pitch);
	float cosYaw = cosf(yaw);
	float sinYaw = sinf(yaw);

	float x[] = { -cosYaw, 0, sinYaw };
	float y[] = { sinYaw * sinPitch, -cosPitch, cosYaw * sinPitch };
	float z[] = { sinYaw * cosPitch, sinPitch, cosPitch * cosYaw };

	result[0] = x[0];
	result[1] = y[0];
	result[2] = z[0];
	result[3] = 0.0f;
	result[4] = x[1];
	result[5] = y[1];
	result[6] = z[1];
	result[7] = 0.0f;
	result[8] = x[2];
	result[9] = y[2];
	result[10] = z[2];
	result[11] = 0.0f;
	result[12] = 0.0f;
	result[13] = 0.0f;
	result[14] = 0.0f;
	result[15] = 1.0f;

	translateMatrix(result, -eye[0], -eye[1], -eye[2]);
}

