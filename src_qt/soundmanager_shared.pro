TARGET   = SoundManager

QT += xml

INCLUDEPATH += . \
               ../../include \
               LogManager
win32{
INCLUDEPATH += $(OPENAL_HOME)/include \
               ../ExternalLibs/libvorbis/include \
               ../ExternalLibs/libogg/include

LIBS += -L$(OPENAL_HOME)/libs/Win32 \
        -lOpenAL32 \
        -L../ExternalLibs/libvorbis/win32/VS2008/Win32/Release \
        -llibvorbis_static \
        -L../ExternalLibs/libogg/win32/VS2008/Win32/Release \
        -llibogg_static
}

macx {
QMAKE_LFLAGS += -F/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks \
				-F/Library/Frameworks

LIBS += -framework CoreFoundation \
		-framework OpenAL \
        -framework LAME \
		-framework AudioToolbox \
        -lsndfile
}

DEFINES += SOUNDMANAGER_LIB

####################################################################
#           DEFAULT CONFIGURATIONS
####################################################################

ROOT_PROJECT_FOLDER = ..

TEMPLATE = lib
DESTDIR  = ../lib

CONFIG( debug, debug|release ) {
TARGET = $${TARGET}_d
BUILD_NAME = debug
}

CONFIG(release, debug|release) {
BUILD_NAME = release
}

include( soundmanager.pri )

####################################################################
