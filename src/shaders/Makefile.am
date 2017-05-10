CLEANFILES = *.spv
hello_vulkan_shaders_dir = $(datadir)/hello-vulkan
dist_hello_vulkan_shaders__DATA = vert.spv frag.spv

vert.spv: shader.vert
	$(AM_V_GEN)glslangValidator -V $^ -o $@

frag.spv: shader.frag
	$(AM_V_GEN)glslangValidator -V $^ -o $@

