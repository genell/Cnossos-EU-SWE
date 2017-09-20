g++ -I ../../PropagationPath -c ../../Cnossos-EU/TestCnossos.cpp
g++ -I ../../PropagationPath -c ../../Cnossos-EU/CopyToClipboard.cpp
g++ -o ../bin/TestCnossos.exe TestCnossos.o CopyToClipboard.o ../lib/libSimpleXML.a ../lib/libPropagation.a ../bin/libHarmonoise.so

