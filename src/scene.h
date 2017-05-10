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

#include <stdint.h>

#include "vulkan-types.h"

// Default cube materials
const static float CUBE_AMBIENT[] = { 0.15f, 0.15f, 0.15f };
const static float CUBE_DIFFUSE[] = { 1.0f, 1.0f, 1.0f };
const static float CUBE_SPECULAR[] = { 1.0f, 1.0f, 1.0f };
const static float CUBE_SPECULAR_EXP = 10.0f;

// Default scene attributes
const static float EYE[] = { 2.5f, 0.0f, 0.0f };
const static float LIGHT_POS[] = { 1.0f, 0.0f, 0.0f };
const static float LIGHT_COLOR[] = { 1.0f, 1.0f, 1.0f };

const static Vertex CUBE_VERTICES[] = {
	{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, -1.0f,  0.0f }, { 0.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f, -0.5f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
	{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
	{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f } },
	{ {  0.5f, -0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } },
	{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f, -0.5f, -0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } },
	{ {  0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f } },
	{ { -0.5f,  0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f } },
	{ {  0.5f,  0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } },
	{ { -0.5f, -0.5f,  0.5f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f, -1.0f,  0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  0.0f,  1.0f,  0.0f }, { 1.0f, 1.0f } },
	{ {  0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f } },
	{ { -0.5f,  0.5f,  0.5f }, {  0.0f,  1.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } },
	{ { -0.5f, -0.5f, -0.5f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } }
};

const static uint16_t CUBE_INDICES[] = {
	 0,  1,  2,
	 3,  4,  5,
	 6,  7,  8,
	 9, 10, 11,
	12, 13, 14,
	15, 16, 17,
	 0, 18,  1,
	 3, 19,  4,
	 6, 20,  7,
	 9, 21, 10,
	12, 22, 13,
	15, 23, 16
};

