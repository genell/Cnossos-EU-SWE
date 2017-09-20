cd bin 
del /Q *.*
cd ..\lib
del /Q *.*
cd ..\obj
del /Q *.o
call Make.SimpleXML
call Make.Harmonoise
call Make.PropagationPath
call Make.TestCnossos
call Make.CnossosDLL
call Make.CnossosCPP
call Make.CnossosEXT
cd ..\bin
TestCnossos
cd ..


