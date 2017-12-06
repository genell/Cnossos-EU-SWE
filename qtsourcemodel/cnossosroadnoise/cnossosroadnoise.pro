#-------------------------------------------------
#
# Project created by QtCreator 2017-11-17T09:46:53
#
#-------------------------------------------------

QT       -= gui

TARGET = cnossosroadnoise
TEMPLATE = lib

DEFINES += CNOSSOSROADNOISE_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# -- INCLUDES -- #
include(../tinyxml/tinyxml.pri)

#INCLUDEPATH += C:/bin/boost_1_65_1/include/boost-1_65_1
#LIBS += C:\bin\boost_1_65_1\bin.v2\libs\

SOURCES += \
    cnossosroadnoise.cpp \
    roadnoisesegment.cpp \
    roadnoisecatalog.cpp \
    roadnoisevehiclecategory.cpp \
    roadnoiseaux.cpp \
    roadnoiseconst.cpp

HEADERS += \
    cnossosroadnoise.h \
    cnossosroadnoise_global.h \
    roadnoisesegment.h \
    roadnoisecatalog.h \
    roadnoisevehiclecategory.h \
    roadnoiseconst.h \
    roadnoiseaux.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
