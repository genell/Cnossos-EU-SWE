g++ -c ../../SimpleXML/SimpleXML.cpp
g++ -c ../../SimpleXML/xmlparse.c
g++ -c ../../SimpleXML/xmlrole.c
g++ -c ../../SimpleXML/xmltok.c
g++ -c ../../SimpleXML/xmltok_impl.c
g++ -c ../../SimpleXML/xmltok_ns.c
ar rcs ../lib/libSimpleXML.a SimpleXMl.o xmlparse.o xmlrole.o xmltok.o xmltok_impl.o xmltok_ns.o

