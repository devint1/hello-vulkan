# Hello Vulkan

This is a simple application I wrote to learn the basics of the Vulkan API. It
is based primarily on
[Alexander Overvoorde's Vulkan tutorial](https://vulkan-tutorial.com/). I have
made a few modifications to better enforce my understanding, such as writing the
code in C, dynamic lighting, normal maps, and camera movement, to name a few.

![Example](https://github.com/devint1/hello-vulkan/raw/master/example.gif)

## Dependencies

*  Suitable Vulkan-supported graphics driver
*  [Vulkan ICD Loader >= 1.0.46](https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers)
*  [GLFW >= 3.2.1](https://github.com/glfw/glfw)
*  [glslangValidator](https://github.com/KhronosGroup/glslang)

*Note: These are the versions I have tested; the program may work with earlier*
*versions. Feel free to modify* `configure.ac` *and report it in the issues*
*section.*

## Building

Building the program is currently based on the GNU Build System. To build:

1.  `autoreconf -i`
2.  `./configure`
3.  `make`
4.  (Optional) `sudo make install`

Once built, the program can be run from `./src/hello-vulkan`, or simply
`hello-vulkan` if the install prefix is in your PATH.

## Controls

*  **W:** Camera forward
*  **S:** Camera reverse
*  **A:** Camera left
*  **D:** Camera right
*  **Q:** Camera up
*  **Z:** Camera down
*  **NUM8:** Light -x
*  **NUM2:** Light +x
*  **NUM4:** Light -z
*  **NUM6:** Light +z
*  **NUM7:** Light -y
*  **NUM1:** Light +y
*  **ESC:** Quit program
*  **Left Mouse:** Rotate camera
*  **Right Mouse:** Rotate model

## Command Line Options

A number of command line options are available by running `hello-vulkan -?` or
`hello-vulkan --help`. A notable option is the interactive console
(`hello-vulkan -i`), which allows modifying several scene/material attributes
on-the-fly.

## Special Thanks

*  Alexander Overvoorde for his awesome Vulkan tutorial.
*  Bricks'n'Tiles, [textures](http://www.bricksntiles.com/textures/).

