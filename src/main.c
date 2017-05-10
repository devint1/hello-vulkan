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

#include <GLFW/glfw3.h>

#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#include "console.h"
#include "glfw-controls.h"
#include "vulkan-draw.h"
#include "vulkan-lifecycle.h"

#define DEFAULT_WIDTH 1024
#define DEFAULT_HEIGHT 768

static void printHelp() {
	printf("Usage: hello-vulkan [options]\n\n"
		   " -w, --width <pixels>\tSet resolution width. Default is %d.\n"
		   " -h, --height <pixels>\tSet resolution height. Default is %d.\n"
		   " -f, --fullscreen\tEnable fullscreen mode. Width and height are\n"
		   "\t\t\tset to the monitor's resolution if not provided.\n"
		   " -v, --novsync\t\tDisable VSync.\n"
		   " -i, --interactive\tLaunch in interactive mode.\n"
		   " -r, --framerate\tDisplay framerate every second. Ignored in\n"
		   "\t\t\tinteractive mode.\n"
		   " -?, --help\t\tDisplay this help.\n", DEFAULT_WIDTH, DEFAULT_HEIGHT);
	exit(0);
}

static void parseArgs(int argc, char* const *argv, int *width, int *height,
			   int *fullscreen, int *noVsync, int *interactive, int *framerate) {

	char c;
	static struct option longOptions[] = {
		{ "width", required_argument, NULL, 'w' },
		{ "height", required_argument, NULL, 'h' },
		{ "fullscreen", no_argument, NULL, 'f' },
		{ "novsync", no_argument, NULL, 'v' },
		{ "interactive", no_argument, NULL, 'i' },
		{ "framerate", no_argument, NULL, 'r' },
		{ "help", no_argument, NULL, '?' }
	};

	while ((c = getopt_long(argc, argv, "w:h:fvir?", longOptions, NULL)) != -1) {
		switch(c) {
			case 'w':
				*width = atoi(optarg);
				if (!*width) {
					fprintf(stderr, "Invalid width value: %s\n", optarg);
					exit(1);
				}
				break;
			case 'h':
				*height = atoi(optarg);
				if (!*height) {
					fprintf(stderr, "Invalid height value: %s\n", optarg);
					exit(1);
				}
				break;
			case 'f':
				*fullscreen = 1;
				break;
			case 'v':
				*noVsync = 1;
				break;
			case 'i':
				*interactive = 1;
				break;
			case 'r':
				*framerate = 1;
				break;
			case '?':
				printHelp();
				break;
		}
	}
}

int main(int argc, char **argv) {
	int width = DEFAULT_WIDTH, height = DEFAULT_HEIGHT, fullscreen = 0,
		noVsync = 0, interactive = 0, enableFramerate = 0;
	unsigned long long nframes = 0;
	double framerate;
	struct timeval tv, start;

	parseArgs(argc, argv, &width, &height, &fullscreen, &noVsync, &interactive,
		&enableFramerate);

	// Initialize GLFW
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	GLFWmonitor *monitor = NULL;
	if (fullscreen) {
		monitor = glfwGetPrimaryMonitor();
		if (width == DEFAULT_WIDTH && height == DEFAULT_HEIGHT && monitor) {
			const GLFWvidmode *mode = glfwGetVideoMode(monitor);
			width = mode->width;
			height = mode->height;
		}
	}
	GLFWwindow* window = glfwCreateWindow(width, height, "Hello Vulkan",
		monitor, NULL);

	// Initialize Vulkan
	VkContext context = {};
	if (!initVulkan(window, &context, !noVsync)) {
		fprintf(stderr, "Vulkan initialization failed.\n");
		destroyVulkan(&context);
		return 1;
	};
	UBOAttributes uboAttributes = initializeUBOAttributes(width, height);

	// Set up the console, if applicable
	ConsoleArgs args = { &uboAttributes, window, &framerate };
	if (interactive) {
		pthread_t thread;
		pthread_create(&thread, NULL, consoleLoop, &args);
	}

	// Record start time for framerate calculation
	gettimeofday(&start, NULL);
	int lastSec = start.tv_sec;

	// Main loop
	while(!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		updateUniformBuffer(window, &uboAttributes, &context);
		drawFrame(&context);

		if (enableFramerate || interactive) {
			++nframes;

			// Account for overflow
			if (nframes == 0) {
				gettimeofday(&start, NULL);
			}

			gettimeofday(&tv, NULL);
			int diffSec = tv.tv_sec - start.tv_sec;
			int diffUsec = tv.tv_usec - start.tv_usec;
			double seconds = diffSec + diffUsec * 0.000001;
			framerate = nframes / seconds;
			if (tv.tv_sec != lastSec && enableFramerate && !interactive) {
				printf("FPS: %f\n", nframes / seconds);
				lastSec = tv.tv_sec;
			}
		}
	}

	// Clean up
	destroyVulkan(&context);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

