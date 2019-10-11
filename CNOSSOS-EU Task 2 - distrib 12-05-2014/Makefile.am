#
# CNOSSOS libraries
#

lib_LTLIBRARIES = libHarmonoise.la libSimpleXML.la libPropagation.la

libSimpleXML_la_SOURCES = \
  SimpleXML/SimpleXML.cpp \
  SimpleXML/SimpleXML.h \
  SimpleXML/ascii.h \
  SimpleXML/asciitab.h \
  SimpleXML/expat.h \
  SimpleXML/expat_external.h \
  SimpleXML/iasciitab.h \
  SimpleXML/internal.h \
  SimpleXML/latin1tab.h \
  SimpleXML/nametab.h \
  SimpleXML/utf8tab.h \
  SimpleXML/xmlparse.c \
  SimpleXML/xmlrole.c \
  SimpleXML/xmlrole.h \
  SimpleXML/xmltok.c \
  SimpleXML/xmltok.h \
  SimpleXML/xmltok_impl.c \
  SimpleXML/xmltok_impl.h \
  SimpleXML/xmltok_ns.c
libSimpleXML_la_LDFLAGS = -version-info 0:0:0

libHarmonoise_la_SOURCES = \
  HarmonoiseP2P/PointToPoint.cpp \
  HarmonoiseP2P/PointToPoint.hpp \
  HarmonoiseP2P/PointToPointEx.hpp
libHarmonoise_la_LDFLAGS = -version-info 0:0:0

libPropagation_la_SOURCES = \
  PropagationPath/CalculationMethod.cpp \
  PropagationPath/CalculationMethod.h \
  PropagationPath/CNOSSOS-2018.cpp \
  PropagationPath/CNOSSOS-2018.h \
  PropagationPath/ElementarySource.h \
  PropagationPath/ErrorMessage.h \
  PropagationPath/Geometry3D.h \
  PropagationPath/ISO-9613-2.cpp \
  PropagationPath/ISO-9613-2.h \
  PropagationPath/JRC-2012.cpp \
  PropagationPath/JRC-2012.h \
  PropagationPath/JRC-draft-2010.cpp \
  PropagationPath/JRC-draft-2010.h \
  PropagationPath/Material.cpp \
  PropagationPath/Material.h \
  PropagationPath/MeanPlane.cpp \
  PropagationPath/MeanPlane.h \
  PropagationPath/MeteoCondition.cpp \
  PropagationPath/MeteoCondition.h \
  PropagationPath/PathParseXML.cpp \
  PropagationPath/PathParseXML.h \
  PropagationPath/PathResult.cpp \
  PropagationPath/PathResult.h \
  PropagationPath/PropagationPath.cpp \
  PropagationPath/PropagationPath.h \
  PropagationPath/ReferenceObject.cpp \
  PropagationPath/ReferenceObject.h \
  PropagationPath/SelectMethod.cpp \
  PropagationPath/SourceGeometry.cpp \
  PropagationPath/SourceGeometry.h \
  PropagationPath/Spectrum.cpp \
  PropagationPath/Spectrum.h \
  PropagationPath/SystemClock.cpp \
  PropagationPath/SystemClock.h \
  PropagationPath/VerticalExt.h
libPropagation_la_LDFLAGS = -version-info 0:0:0

#
# GNU Octave module
#
if WANT_OCTAVE
  bin_PROGRAMS = cnossos_flat cnossos_full
  cnossos_flat_SOURCES = octave/cnossos_flat.cpp
  cnossos_flat_CPPFLAGS = $(OCTAVE_CPPFLAGS) -I$(top_srcdir)/PropagationPath
  # cnossos_flat_LDADD = libHarmonoise.la libPropagation.la
  cnossos_flat_LINK = $(MKOCTPATH) -g -v $(OCTAVE_CPPFLAGS) -o $@ -L.libs -lPropagation -lHarmonoise
  
  cnossos_full_SOURCES = octave/cnossos_full.cpp
  cnossos_full_CPPFLAGS = $(OCTAVE_CPPFLAGS) -I$(top_srcdir)/PropagationPath
  # cnossos_full_LDADD = libHarmonoise.la libPropagation.la
  cnossos_full_LINK = $(MKOCTPATH) -g -v $(OCTAVE_CPPFLAGS) -o $@ -L.libs -lPropagation -lHarmonoise
endif
