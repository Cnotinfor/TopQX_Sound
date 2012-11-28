/*!
	\class CnotiAudio::SoundBase
	\brief Base class to all sound classes

	\version 2.0
	\data 14-11-2008
	\file SoundBase.h
*/
#if !defined(_CNOTISOUND_H)
#define _CNOTISOUND_H

#include "CnotiAudio.h"
#include "soundmanager_global.h"

//
// QT
//
#include <QTime>
#include <QThread>
#include <QMutex>

//
// OpenAL FrameWork
//
#ifdef _WIN32
#include "openal\win32\aldlist.h"
#include "openal\win32\CWaves.h"
#include "openal\win32\Framework.h"
#else
#include "openal/MacOSX/MyOpenALSupport.h"
#endif

namespace CnotiAudio
{
	#define CS_REFRESH				(20)

//	class XmlSoundHandler;
	class Melody;
	class SoundManager;

	class SoundBase: public QThread
	{
		Q_OBJECT

	public:	
		SoundBase(const QString name = "unknown");
		SoundBase(const SoundBase &other);
		SoundBase(SoundBase &other, const QString newName);
		~SoundBase();

		virtual bool load(const QString filename)=0;
		virtual void release()=0;

		virtual bool playSound( bool loop = false, bool blockSignal = false )=0;
		virtual bool pauseSound()=0;
		virtual bool stopSound()=0;

		virtual bool isEmpty()=0;

		virtual bool compareSound(SoundBase* second)=0;
		virtual float percentPlay()=0;

		virtual bool save(const QString filename)=0;
		virtual bool saveWav(const QString filename);
		virtual bool saveMp3(const QString filename, int minimumRate, bool deleteWav=true);

		virtual bool isPlaying();
		virtual bool isStopped();
		virtual bool isPaused();

		virtual void setVolume(float intensity);

		void connectToSoundManager();
		
		void setSourcePosition(float x, float y, float z);

		QString getName();

		CnotiErrorSound getLastError();
//		TypeSound getType();
		float getIntensity();
		
		virtual short* getData();
		virtual unsigned long getSize();
		ALint getFrequency();
		void setFrequency(ALint frequency);

		float getSourceX();
		float getSourceY();
		float getSourceZ();

		float getDuration();

	signals:
/*!
	This signal is emitted when the sound is stopped.
*/
		void soundStopped(QString name);
/*!
	This signal is emitted when the sound is paused.
*/
		void soundPlaying(QString name);
/*!
	This signal is emitted when the sound is paused.
*/
		void soundPaused(QString name);

	protected:
		SoundManager*               _soundMgr;			// Pointer to SoundManager
		QString                     _name;              // Sound name
		CnotiErrorSound				_lastError;         // Last sound error
		int							_totalDuration;     // Duration of the sound
		
		QTime*						_timer;
		int							_currTime;
		int							_pauseTime;

		// QtThread	
		QMutex						_mutex;
		QMutex						_dataMutex;
		volatile bool				_flagThreadSoundStopped;
		volatile bool				_stopped;

		bool						_loop;              // Keeps if sound is to bee played in a loop

		short*						_data;              // Keeps the sound data
		unsigned long				_size;				// Keeps the data size
		ALint						_iFrequency;		// Sound frequency
		ALfloat						_sourcePos[3];      // Sound position
		ALuint						_uiSource;          // Source to play sound
		float						_intensity;         // Sound volume

		QString                     _logFile;           // 

	// Functions
		virtual void run();
		virtual void update()=0;
	};
}

#endif  //_CNOTISOUND_H
