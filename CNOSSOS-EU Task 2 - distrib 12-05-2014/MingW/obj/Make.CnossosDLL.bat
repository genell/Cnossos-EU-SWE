g++ -I ../../PropagationPath -c ../../CnossosPropagation/CnossosPropagation.cpp
g++ -shared -o ../bin/libPropagation.so CnossosPropagation.o ../lib/libPropagation.a ../lib/libSimpleXML.a ../bin/libHarmonoise.so
g++ -I ../../CnossosPropagation -c ../../TestCnossosDLL/TestCnossosDLL.cpp
g++ -o ../bin/TestCnossosDLL.exe TestCnossosDLL.o ../bin/libPropagation.so ../bin/libHarmonoise.so
