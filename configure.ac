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

AC_INIT([hello-vulkan], [1.0], [devin.tuchsen@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AM_SILENT_RULES([yes])
AC_PROG_CC
AM_PROG_CC_C_O
AX_PTHREAD
AC_CONFIG_HEADERS([config.h])
PKG_CHECK_MODULES([VULKAN], [vulkan >= 1.0.46])
PKG_CHECK_MODULES([GLFW3], [glfw3 >= 3.2.1])
AC_CHECK_PROG([HAVE_GLSLANG], [glslangValidator], [yes])
if test x"$HAVE_GLSLANG" != x"yes"; then
	AC_MSG_ERROR([glslangValidator required to compile shaders.])
fi
AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/shaders/Makefile
	textures/Makefile
])
AC_OUTPUT

