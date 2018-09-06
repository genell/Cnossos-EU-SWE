.PHONY: clean init tinyxml
.SUFFIXES:
.SUFFIXES: .c .cpp .o

build_root = .
dist_dir = $(build_root)/dist
build_dir = $(build_root)/build
bld_dirs = $(dist_dir) $(build_dir)

# prefix build/dist dirs to dependencies, based on variable suffix
deps = $(patsubst %, $(build_dir)/%,$(filter %.o %.a,$(1))) $(patsubst %, $(dist_dir)/%,$(filter %.so,$(1)))

deliverables = CnossosConsole #libIndustrialNoise.so libRailNoise.so libRoadNoise.so

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
consoleapp = $(CXX) -o $@ $^ -lcurses  -Wl,-rpath=.

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
roadnoise: $(build_dir)/libRoadNoise.a
ROADNOISE_DEPS = CNOSSOS_ROADNOISE_DLL.o CNOSSOS_ROADNOISE_DLL_AUX.o CNOSSOS_ROADNOISE_DLL_CONST.o CNOSSOS_ROADNOISE_DLL_DATA.o
$(build_dir)/libRoadNoise.a: $(call deps,$(ROADNOISE_DEPS))
	$(staticlib);
	cp source/CNOSSOS_ROADNOISE_DLL/*.xml $(dist_dir);

#
# railnoise
#
railnoise: $(build_dir)/libRailNoise.a
RAILNOISE_DEPS = CNOSSOS_RAILNOISE_DLL.o CNOSSOS_RAILNOISE_DLL_AUX.o CNOSSOS_RAILNOISE_DLL_CONST.o CNOSSOS_RAILNOISE_DLL_DATA.o
$(build_dir)/libRailNoise.a: $(call deps,$(RAILNOISE_DEPS))
	$(staticlib)
	cp source/CNOSSOS_RAILNOISE_DLL/*.xml $(dist_dir);

#
# industrial noise
#
industrialnoise: $(build_dir)/libIndustrialNoise.a
INDNOISE_DEPS = CNOSSOS_INDUSTRIAL_NOISE_DLL.o CNOSSOS_IND_DATA.o
$(build_dir)/libIndustrialNoise.a: $(call deps,$(INDNOISE_DEPS))
	$(staticlib)
	cp source/CNOSSOS_INDUSTRIAL_NOISE_DLL/*.xml $(dist_dir);

#
# Cnossos console app
#
cnossosconsole: $(dist_dir)/CnossosConsole
CNOSCON_DEPS = CNOSSOS_DLL_CONSOLE.o CNOSSOS_AUX.o tinyxml.a libIndustrialNoise.a libRailNoise.a libRoadNoise.a
$(dist_dir)/CnossosConsole: $(call deps,$(CNOSCON_DEPS))
	$(consoleapp);

