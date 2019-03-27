.PHONY: clean init
.SUFFIXES:
.SUFFIXES: .c .cpp .o

build_root = .
dist_dir = $(build_root)/dist
build_dir = $(build_root)/build
bld_dirs = $(dist_dir) $(build_dir)

# prefix build/dist dirs to dependencies, based on variable suffix
deps = $(patsubst %, $(build_dir)/%,$(filter %.o %.a,$(1))) $(patsubst %, $(dist_dir)/%,$(filter %.so,$(1)))

deliverables = TestCnossos TestCnossosLib TestCnossosCPP TestCnossosEXT libPropagation.so libHarmonoise.so

all: $(patsubst %, $(dist_dir)/%,$(deliverables)) harmonoisep2p

# create build directories
$(bld_dirs):
	mkdir $@;

clean:
	for dir in $(bld_dirs); do \
		rm -rf $$dir; \
	done

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
# SimpleXML
#
SIMPLEXML_DEPS = SimpleXML.o xmlparse.o xmlrole.o xmltok.o xmltok_impl.o xmltok_ns.o
$(build_dir)/libSimpleXML.a: $(call deps,$(SIMPLEXML_DEPS))
	$(staticlib)

#
# HarmonoiseP2P
#
harmonoisep2p: $(dist_dir)/libHarmonoise.so $(build_dir)/libHarmonoise.a
HARMONOISE_DEPS = PointToPoint.o
$(dist_dir)/libHarmonoise.so: $(call deps,$(HARMONOISE_DEPS))
	$(sharedlib)
$(build_dir)/libHarmonoise.a: $(call deps,$(HARMONOISE_DEPS))
	$(staticlib)

#
# PropagationPath
#
propagationpath: $(build_dir)/libPropagation.a
PROPPATH_DEPS = CalculationMethod.o CNOSSOS-2018.o ISO-9613-2.o JRC-2012.o JRC-draft-2010.o Material.o MeanPlane.o MeteoCondition.o PathParseXML.o PathResult.o PropagationPath.o ReferenceObject.o SelectMethod.o SourceGeometry.o Spectrum.o SystemClock.o unixgcc.o
$(build_dir)/libPropagation.a: $(call deps,$(PROPPATH_DEPS))
	$(staticlib)

#
# TestCnossos
#
testcnossos: $(dist_dir)/TestCnossos
TCNO_DEPS = TestCnossos.o libSimpleXML.a libPropagation.a libHarmonoise.so
$(dist_dir)/TestCnossos: $(call deps,$(TCNO_DEPS))
	$(consoleapp)

#
# CnossosDLL
#
testcnossoslib: $(dist_dir)/TestCnossosLib
CNOSLIB_DEPS = CnossosPropagation.o libPropagation.a libSimpleXML.a libHarmonoise.so
$(dist_dir)/libPropagation.so: $(call deps,$(CNOSLIB_DEPS))
	$(sharedlib)
TCNOSLIB_DEPS = TestCnossosDLL.o libPropagation.so libHarmonoise.so
$(dist_dir)/TestCnossosLib: $(call deps,$(TCNOSLIB_DEPS))
	$(consoleapp)

#
# CnossosCPP
#
testcnossoscpp: $(dist_dir)/TestCnossosCPP
TCNOCPP_DEPS = TestCnossosCPP.o libPropagation.so libHarmonoise.so
$(dist_dir)/TestCnossosCPP: $(call deps,$(TCNOCPP_DEPS))
	$(consoleapp)

#
# CnossosEXT
#
testcnossosext: $(dist_dir)/TestCnossosEXT
TCNOEXT_DEPS = TestCnossosEXT.o libPropagation.so libHarmonoise.so
$(dist_dir)/TestCnossosEXT: $(call deps,$(TCNOEXT_DEPS))
	$(consoleapp)
