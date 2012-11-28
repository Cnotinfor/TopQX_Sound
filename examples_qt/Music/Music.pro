#-------------------------------------------------
#
# Project created by QtCreator 2011-01-28T14:49:20
#
#-------------------------------------------------

QT       += core gui

TARGET = Music
TEMPLATE = app

INCLUDEPATH += $(OPENAL_HOME)/include \
			   ../../ExternalLibs/libvorbis/include \
			   ../../ExternalLibs/libogg/include \

LIBS += -L../../lib

CONFIG( debug, debug|release ) {
	TARGET = $${TARGET}_d
	BUILD_NAME = debug
	LIBS += -lSoundManager_d
}
CONFIG( release, debug|release ) {
	BUILD_NAME = release
	LIBS += -lSoundManager
}

SOURCES += main.cpp\
		MusicExample.cpp

HEADERS  += MusicExample.h

FORMS    += MusicExample.ui
