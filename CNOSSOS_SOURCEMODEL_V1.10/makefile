.PHONY: clean init tinyxml
.SUFFIXES:
.SUFFIXES: .c .cpp .o

build_root = .
dist_dir = $(build_root)/dist
build_dir = $(build_root)/build
bld_dirs = $(dist_dir) $(build_dir)

# prefix build/dist dirs to dependencies, based on variable suffix
deps = $(patsubst %, $(build_dir)/%,$(filter %.o %.a,$(1))) $(patsubst %, $(dist_dir)/%,$(filter %.so,$(1)))

deliverables = libIndustrialNoise.so libRailNoise.so libRoadNoise.so CnossosConsole

all: $(patsubst %, $(dist_dir)/%,$(deliverables))

# create build directories
$(bld_dirs):
	mkdir $@;

clean:
	for dir in $(bld_dirs); do \
		rm -rf $$dir; \
	done

# Source search folders
VPATH = source/tinyxml:source/CNOSSOS_ROADNOISE_DLL:source/CNOSSOS_RAILNOISE_DLL:source/CNOSSOS_INDUSTRIAL_NOISE_DLL:source/CNOSSOS_DLL_CONSOLE
CXXFLAGS = -fPIC

$(build_dir)/%.o: %.cpp | $(bld_dirs)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(build_dir)/%.o: %.c | $(bld_dirs)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

staticlib = ar rcs $@ $^
sharedlib = $(CXX) -shared -o $@ $^
consoleapp = $(CXX) -o $@ $^ -lcurses -L$(dist_dir) -Wl,-R. -lRoadNoise -lRailNoise -lIndustrialNoise

#
# tinyxml
#
tinyxml: $(build_dir)/tinyxml.a
TINYXML_DEPS = tinyxml.o tinyxmlparser.o tinyxmlerror.o tinystr.o
$(build_dir)/tinyxml.a: $(call deps,$(TINYXML_DEPS))
	$(staticlib)
#	$(MAKE) -C source/tinyxml;
#	cp source/tinyxml/tinyxml.a $(build_dir)

#
# Road noise
#
roadnoise: $(dist_dir)/libRoadNoise.so
ROADNOISE_DEPS = CNOSSOS_ROADNOISE_DLL.o CNOSSOS_ROADNOISE_DLL_AUX.o CNOSSOS_ROADNOISE_DLL_CONST.o CNOSSOS_ROADNOISE_DLL_DATA.o
$(dist_dir)/libRoadNoise.so: $(call deps,$(ROADNOISE_DEPS))
	$(sharedlib);
	cp source/CNOSSOS_ROADNOISE_DLL/*.xml $(dist_dir);

#
# railnoise
#
railnoise: $(dist_dir)/libRailNoise.so
RAILNOISE_DEPS = CNOSSOS_RAILNOISE_DLL.o CNOSSOS_RAILNOISE_DLL_AUX.o CNOSSOS_RAILNOISE_DLL_CONST.o CNOSSOS_RAILNOISE_DLL_DATA.o
$(dist_dir)/libRailNoise.so: $(call deps,$(RAILNOISE_DEPS))
	$(sharedlib)
	cp source/CNOSSOS_RAILNOISE_DLL/*.xml $(dist_dir);

#
# industrial noise
#
industrialnoise: $(dist_dir)/libIndustrialNoise.so
INDNOISE_DEPS = CNOSSOS_INDUSTRIAL_NOISE_DLL.o CNOSSOS_IND_DATA.o
$(dist_dir)/libIndustrialNoise.so: $(call deps,$(INDNOISE_DEPS))
	$(sharedlib)
	cp source/CNOSSOS_INDUSTRIAL_NOISE_DLL/*.xml $(dist_dir);

#
# Cnossos console app
#
cnossosconsole: $(dist_dir)/CnossosConsole | roadnoise railnoise industrialnoise
CNOSCON_DEPS = CNOSSOS_DLL_CONSOLE.o CNOSSOS_AUX.o tinyxml.a #libIndustrialNoise.so libRailNoise.so libRoadNoise.so
$(dist_dir)/CnossosConsole: $(call deps,$(CNOSCON_DEPS))
	$(consoleapp);

