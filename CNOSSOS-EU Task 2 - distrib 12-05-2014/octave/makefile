.PHONY: clean init deps
.SUFFIXES:
.SUFFIXES: .c .cpp .o

build_root = .
dist_dir = $(build_root)/dist
build_dir = $(build_root)/build
# bld_dirs = $(dist_dir) $(build_dir)

octave_root = /var/lib/flatpak/app/org.octave.Octave/x86_64/stable/a7d9267c2f66925be4f741eeb9dff7b771eea95849223205ebca7ffbc6b2adb2/files
octave_include = $(octave_root)/include/octave-5.1.0

# prefix build/dist dirs to dependencies, based on variable suffix
deps = $(patsubst %, $(build_dir)/%,$(filter %.o %.a,$(1))) $(patsubst %, $(dist_dir)/%,$(filter %.so,$(1)))

deliverables = cnossos-flat cnossos-full

all: $(deliverables)

# create build directories
$(bld_dirs):
	mkdir $@;

clean:
	for dir in $(bld_dirs); do \
		rm -rf $$dir; \
	done

# dependent on 
cnosdeps:
	(cd ../linux && make harmonoisep2p propagationpath)

# Source search folders
VPATH = ../system:../SimpleXML:../HarmonoiseP2P:../PropagationPath:../Cnossos-EU:../CnossosPropagation:../TestCnossosDLL:../TestCnossosCPP:../TestCnossosEXT
CXXFLAGS = -fPIC -Wall -O3 -I ../PropagationPath

$(build_dir)/%.o: %.cpp | $(bld_dirs)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

$(build_dir)/%.o: %.c | $(bld_dirs)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c -o $@ $<

staticlib = ar rcs $@ $^
sharedlib = $(CXX) -g -shared -o $@ $^
consoleapp = $(CXX) -g -o $@ $^ -lcurses

#
# cnossos_flat()
#
cnossos-flat: cnosdeps 
	$(octave_root)/bin/mkoctfile -v -L../linux/build -lPropagation: -lHarmonise: -I$(octave_include) -I../PropagationPath cnossos_flat.cpp

#
#
#
cnossos-full: cnosdeps
	$(octave_root)/bin/mkoctfile -v -L../linux/build -lPropagation: -lHarmonise: -I$(octave_include) -I../PropagationPath cnossos_full.cpp