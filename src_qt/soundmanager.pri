HEADERS +=  BladeMP3EncDLL.h \
			Music.h \
			Melody.h \
			singleton.h \
			SoundManager.h \
			SourcePool.h \
			XmlSoundHandler.h \
			capturethread.h \
			CnotiAudio.h \
			note.h \
			sound.h \
			sample.h \
			notemisc.h \
			soundBase.h \
			stream.h \
			LogManager/logmanager.h \
			LogManager/logmanager_global.h

win32 {
HEADERS +=  openal/win32/aldlist.h \
			openal/win32/CWaves.h \
			openal/win32/Framework.h \
			openal/win32/LoadOAL.h
}

mac{
HEADERS += openal/MacOSX/MyOpenALSupport.h
}

SOURCES +=  Music.cpp \
			Melody.cpp \
			Sample.cpp \
			Sound.cpp \
			SoundManager.cpp \
			SourcePool.cpp \
			Stream.cpp \
			XmlSoundHandler.cpp \
			capturethread.cpp \
			note.cpp \
			notemisc.cpp \
			soundBase.cpp \
			LogManager/logmanager.cpp

win32 {
SOURCES +=  openal/win32/aldlist.cpp \
			openal/win32/CWaves.cpp \
			openal/win32/Framework.cpp \
			openal/win32/LoadOAL.cpp
}

mac{
SOURCES += openal/MacOSX/MyOpenALSupport.cpp
}
