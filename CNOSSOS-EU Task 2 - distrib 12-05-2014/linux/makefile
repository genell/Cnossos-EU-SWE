GCC = g++

root_dir = .
bin_dir = $(root_dir)/bin
lib_dir = $(root_dir)/lib
obj_dir = $(root_dir)/obj
bld_dirs = $(bin_dir) $(lib_dir) $(obj_dir)

default: simplexml harmonisep2p propagationpath testcnossos testcnossoslib testcnossoscpp testcnossosext 

init:
	mkdir bin; mkdir lib; mkdir obj;

clean:
	for dir in $(bld_dirs); do \
		rm -rf $$dir; \
	done

VPATH = ../system:../SimpleXML:../HarmonoiseP2P:../PropagationPath:../Cnossos-EU:../CnossosPropagation:../TestCnossosDLL:../TestCnossosCPP:../TestCnossosEXT
CFLAGS = -fPIC -I ../PropagationPath

$(obj_dir)/%.o: %.cpp |
	$(GCC) -c -o $@ $< $(CFLAGS)

$(obj_dir)/%.o: %.c
	$(GCC) -c -o $@ $< $(CFLAGS)

#
# SimpleXML
#
simplexml: $(lib_dir)/libSimpleXML.a
SIMPLEXML_OBJ = SimpleXML.o xmlparse.o xmlrole.o xmltok.o xmltok_impl.o xmltok_ns.o
$(lib_dir)/libSimpleXML.a: $(patsubst %, $(obj_dir)/%,$(SIMPLEXML_OBJ))
	ar rcs $@ $^

#
# HarmoniseP2P
#
harmonisep2p: $(bin_dir)/libHarmonise.so
HARMONISE_OBJ = PointToPoint.o
$(bin_dir)/libHarmonise.so: $(patsubst %, $(obj_dir)/%,$(HARMONISE_OBJ))
	$(GCC) -shared -o $@ $^

#
# PropagationPath
#
propagationpath: $(lib_dir)/libPropagation.a
PROPPATH_OBJ = CalculationMethod.o ISO-9613-2.o JRC-2012.o JRC-draft-2010.o Material.o MeanPlane.o MeteoCondition.o PathParseXML.o PathResult.o PropagationPath.o ReferenceObject.o SelectMethod.o SourceGeometry.o Spectrum.o SystemClock.o unixgcc.o
$(lib_dir)/libPropagation.a: $(patsubst %, $(obj_dir)/%,$(PROPPATH_OBJ))
	ar rcs $@ $^

#
# TestCnossos
#
testcnossos: $(bin_dir)/TestCnossos
TCNO_OBJ = TestCnossos.o #CopyToClipboard.o
TCNO_LIB = libSimpleXML.a libPropagation.a
TCNO_SLIB = libHarmonise.so
$(bin_dir)/TestCnossos: $(patsubst %, $(obj_dir)/%,$(TCNO_OBJ)) $(patsubst %, $(lib_dir)/%,$(TCNO_LIB)) $(patsubst %, $(bin_dir)/%,$(TCNO_SLIB))
	g++ -o $@ $^ -lcurses

#
# CnossosDLL
#
testcnossoslib: $(bin_dir)/TestCnossosLib
CNOSLIB_OBJ = CnossosPropagation.o
CNOSLIB_LIB = libPropagation.a libSimpleXML.a
CNOSLIB_SLIB = libHarmonise.so
$(bin_dir)/libPropagation.so: $(patsubst %, $(obj_dir)/%,$(CNOSLIB_OBJ)) $(patsubst %, $(lib_dir)/%,$(CNOSLIB_LIB)) $(patsubst %, $(bin_dir)/%,$(CNOSLIB_SLIB))
	$(GCC) -shared -o $@ $^ 
TCNOSLIB_OBJ = TestCnossosDLL.o
TCNOSLIB_SLIB = libPropagation.so libHarmonise.so
$(bin_dir)/TestCnossosLib: $(patsubst %, $(obj_dir)/%,$(TCNOSLIB_OBJ)) $(patsubst %, $(bin_dir)/%,$(TCNOSLIB_SLIB))
	g++ -o $@ $^ -lcurses

#
# CnossosCPP
#
testcnossoscpp: $(bin_dir)/TestCnossosCPP
TCNOCPP_OBJ = TestCnossosCPP.o
TCNOCPP_SLIB = libPropagation.so libHarmonise.so
$(bin_dir)/TestCnossosCPP: $(patsubst %, $(obj_dir)/%,$(TCNOCPP_OBJ)) $(patsubst %, $(bin_dir)/%,$(TCNOCPP_SLIB))
	g++ -o $@ $^ -lcurses

#
# CnossosEXT
#
testcnossosext: $(bin_dir)/TestCnossosEXT
TCNOEXT_OBJ = TestCnossosEXT.o
TCNOEXT_SLIB = libPropagation.so libHarmonise.so
$(bin_dir)/TestCnossosEXT: $(patsubst %, $(obj_dir)/%,$(TCNOEXT_OBJ)) $(patsubst %, $(bin_dir)/%,$(TCNOEXT_SLIB))
	g++ -o $@ $^ -lcurses
