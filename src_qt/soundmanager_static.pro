TARGET   = SoundManager
PRIFILE  = soundmanager.pri
INCLUDEPATH += . \
			   ../../include \
			   $(OPENAL_HOME)/include \
			   ../ExternalLibs/libvorbis/include \
			   ../ExternalLibs/libogg/include \
			   LogManager

LIBS += -L$(OPENAL_HOME)/libs/Win32 \
		-lOpenAL32 \
		-L../ExternalLibs/libvorbis/win32/VS2008/Win32/Release \
		-llibvorbis_static \
		-L../ExternalLibs/libogg/win32/VS2008/Win32/Release \
		-llibogg_static

QT += xml

####################################################################
#           DEFAULT CONFIGURATIONS
####################################################################

TEMPLATE = lib
DESTDIR  = ../lib
CONFIG  += staticlib create_prl

CONFIG( debug, debug|release ) {
TARGET = $${TARGET}_d
BUILD_NAME = debug
}

CONFIG(release, debug|release) {
BUILD_NAME = release
}

INCLUDEPATH += ../../tmp/GeneratedFiles/$$TARGET/$$BUILD_NAME \
../../tmp/GeneratedFiles/$$TARGET

OBJECTS_DIR += ../../tmp/GeneratedFiles/$$TARGET/$$BUILD_NAME
MOC_DIR += ../../tmp/GeneratedFiles/$$TARGET/$$BUILD_NAME
UI_DIR += ../../tmp/GeneratedFiles/$$TARGET
RCC_DIR += ../../tmp/GeneratedFiles/$$TARGET


include( $$PRIFILE )

####################################################################
