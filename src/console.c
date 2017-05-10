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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "console.h"

#define VALIDATE_ARG_COUNT(act, exp) if (act != exp) {\
	printf("Requires %d argument(s).\n\n", exp); return 0;}

static void printVec3(const float* const vec) {
	printf("{ %ff, %ff, %ff }\n\n", vec[0], vec[1], vec[2]);
}

static void setAmbient(UBOAttributes *attributes, float r, float g, float b) {
	attributes->sceneAttributes.ambientColor[0] = r;
	attributes->sceneAttributes.ambientColor[1] = g;
	attributes->sceneAttributes.ambientColor[2] = b;
}

static void setDiffuse(UBOAttributes *attributes, float r, float g, float b) {
	attributes->sceneAttributes.diffuseColor[0] = r;
	attributes->sceneAttributes.diffuseColor[1] = g;
	attributes->sceneAttributes.diffuseColor[2] = b;
}

static void setSpecular(UBOAttributes *attributes, float r, float g, float b) {
	attributes->sceneAttributes.specularColor[0] = r;
	attributes->sceneAttributes.specularColor[1] = g;
	attributes->sceneAttributes.specularColor[2] = b;
}

static void setSpecularExp(UBOAttributes *attributes, float e) {
	attributes->sceneAttributes.specularExp = e;
}

static void setEye(UBOAttributes *attributes, float x, float y, float z) {
	attributes->sceneAttributes.eyePos[0] = x;
	attributes->sceneAttributes.eyePos[1] = y;
	attributes->sceneAttributes.eyePos[2] = z;
}

static void setLightPos(UBOAttributes *attributes, float x, float y, float z) {
	attributes->sceneAttributes.lightPos[0] = x;
	attributes->sceneAttributes.lightPos[1] = y;
	attributes->sceneAttributes.lightPos[2] = z;
}

static void setLightColor(UBOAttributes *attributes, float r, float g, float b) {
	attributes->sceneAttributes.lightColor[0] = r;
	attributes->sceneAttributes.lightColor[1] = g;
	attributes->sceneAttributes.lightColor[2] = b;
}

static void printHelp() {
	printf("  Arguments are floating point values.\n"
		   "  Specify a command without arguments to get the current value.\n\n"
		   "  ambient [r] [g] [b]\t\tSet the ambient color.\n"
		   "  diffuse [r] [g] [b]\t\tSet the diffuse color.\n"
		   "  specular [r] [g] [b]\t\tSet the specular color.\n"
		   "  specularExp [e]\t\tSet the specular exponent.\n"
		   "  eye [x] [y] [z]\t\tSet the eye/camera position.\n"
		   "  lightPos [x] [y] [z]\t\tSet the light position.\n"
		   "  lightColor [r] [g] [b]\tSet the light color.\n"
		   "  fps\t\t\t\tDisplay the current framerate.\n"
		   "  quit\t\t\t\tQuit the program.\n"
		   "  help\t\t\t\tDisplay this help.\n\n");
}

static int parseLine(char* line, ConsoleArgs *consoleArgs) {
	char *cmd = strtok(line, " \n");
	if (!cmd) {
		return 0;
	}
	float args[3];
	int i = 0;
	for (; i < 3; ++i) {
		char *arg = strtok(NULL, " "), *endptr;
		if (!arg) {
			break;
		}
		args[i] = strtof(arg, &endptr);
		if (endptr == arg) {
			printf("Invalid argument: %s\n\n", arg);
			return 0;
		}
	}

	if (!strcasecmp("ambient", cmd)) {
		if (i == 0) {
			printVec3(consoleArgs->uboAttributes->sceneAttributes.ambientColor);
			return 0;
		}
		VALIDATE_ARG_COUNT(i, 3);
		setAmbient(consoleArgs->uboAttributes, args[0], args[1], args[2]);
	} else if (!strcasecmp("diffuse", cmd)) {
		if (i == 0) {
			printVec3(consoleArgs->uboAttributes->sceneAttributes.diffuseColor);
			return 0;
		}
		VALIDATE_ARG_COUNT(i, 3);
		setDiffuse(consoleArgs->uboAttributes, args[0], args[1], args[2]);
	} else if (!strcasecmp("specular", cmd)) {
		if (i == 0) {
			printVec3(consoleArgs->uboAttributes->sceneAttributes.specularColor);
			return 0;
		}
		VALIDATE_ARG_COUNT(i, 3);
		setSpecular(consoleArgs->uboAttributes, args[0], args[1], args[2]);
	} else if (!strcasecmp("specularExp", cmd)) {
		if (i == 0) {
			printf("%ff\n\n", consoleArgs->uboAttributes->sceneAttributes.specularExp);
			return 0;
		}
		VALIDATE_ARG_COUNT(i, 1);
		setSpecularExp(consoleArgs->uboAttributes, args[0]);
	} else if (!strcasecmp("eye", cmd)) {
		if (i == 0) {
			printVec3(consoleArgs->uboAttributes->sceneAttributes.eyePos);
			return 0;
		}
		VALIDATE_ARG_COUNT(i, 3);
		setEye(consoleArgs->uboAttributes, args[0], args[1], args[2]);
	} else if (!strcasecmp("lightPos", cmd)) {
		if (i == 0) {
			printVec3(consoleArgs->uboAttributes->sceneAttributes.lightPos);
			return 0;
		}
		VALIDATE_ARG_COUNT(i, 3);
		setLightPos(consoleArgs->uboAttributes, args[0], args[1], args[2]);
	} else if (!strcasecmp("lightColor", cmd)) {
		if (i == 0) {
			printVec3(consoleArgs->uboAttributes->sceneAttributes.lightColor);
			return 0;
		}
		VALIDATE_ARG_COUNT(i, 3);
		setLightColor(consoleArgs->uboAttributes, args[0], args[1], args[2]);
	} else if (!strcasecmp("fps", cmd)) {
		VALIDATE_ARG_COUNT(i, 0);
		printf("FPS: %f\n\n", *consoleArgs->framerate);
	} else if (!strcasecmp("quit", cmd)) {
		VALIDATE_ARG_COUNT(i, 0);
		glfwSetWindowShouldClose(consoleArgs->window, 1);
		return 1;
	} else if (!strcasecmp("help", cmd)) {
		VALIDATE_ARG_COUNT(i, 0);
		printHelp();
	} else {
		printf("Unknown command: %s\nType \"help\" for a list of commands.\n\n",
			cmd);
		return 0;
	}
	return 0;
}

void* consoleLoop(void *args) {
	ConsoleArgs *consoleArgs = (ConsoleArgs*) args;
	int run = 1;
	char *line = NULL;
	size_t lineLen;

	printf("Starting interactive console. Type \"help\" for a list of commands."
		   "\n\n");
	while (run) {
		printf("> ");
		getline(&line, &lineLen, stdin);
		if (parseLine(line, consoleArgs)) {
			run = 0;
		}
	}
	free(line);
}

