# This file is part of Hello Vulkan.
#
# Hello Vulkan is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hello Vulkan is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hello Vulkan.  If not, see <http://www.gnu.org/licenses/>.

SUBDIRS = shaders
bin_PROGRAMS = hello-vulkan
hello_vulkan_CFLAGS = $(VULKAN_CFLAGS) $(GLFW3_CFLAGS) $(PTHREAD_CFLAGS)
hello_vulkan_LDFLAGS = $(VULKAN_LIBS) $(GLFW3_LIBS) $(PTHREAD_LIBS)
hello_vulkan_SOURCES = console.c console.h glfw-controls.c glfw-controls.h \
	main.c maths.c maths.h scene.h vulkan-draw.c vulkan-draw.h \
	vulkan-lifecycle.c vulkan-lifecycle.h vulkan-types.h

