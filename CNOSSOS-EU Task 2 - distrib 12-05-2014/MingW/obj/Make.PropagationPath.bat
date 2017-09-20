gcc -c ../../PropagationPath/CalculationMethod.cpp
gcc -c ../../PropagationPath/ISO-9613-2.cpp
gcc -c ../../PropagationPath/JRC-2012.cpp
gcc -c ../../PropagationPath/JRC-draft-2010.cpp
gcc -c ../../PropagationPath/Material.cpp
gcc -c ../../PropagationPath/MeanPlane.cpp
gcc -c ../../PropagationPath/MeteoCondition.cpp
gcc -c ../../PropagationPath/PathParseXML.cpp
gcc -c ../../PropagationPath/PathResult.cpp
gcc -c ../../PropagationPath/PropagationPath.cpp
gcc -c ../../PropagationPath/ReferenceObject.cpp
gcc -c ../../PropagationPath/SelectMethod.cpp
gcc -c ../../PropagationPath/SourceGeometry.cpp
gcc -c ../../PropagationPath/Spectrum.cpp
gcc -c ../../PropagationPath/SystemClock.cpp
ar rcs ../lib/libPropagation.a CalculationMethod.o ISO-9613-2.o JRC-2012.o JRC-draft-2010.o Material.o MeanPlane.o MeteoCondition.o 
ar rcs ../lib/libPropagation.a PathParseXML.o PathResult.o PropagationPath.o ReferenceObject.o SelectMethod.o SourceGeometry.o Spectrum.o SystemClock.o

