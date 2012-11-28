/*!
	\class CnotiAudio::SoundManager
	\brief Class to manage all type of sounds.

	This cass is Singleton, so only one instance of SoundManager will be available. To get the instance use:

	SoundManger* mySoundManager = CnotiAudio::SoundManager::getSingletonPtr();

	The openal and ogg is initialized in this class. All the sounds are save into a list of SoundBase.
	Before load any sound you must initialize the openal and if the files are ogg you must
	initialize the ogg.

	\version 2.0
	\data 14-11-2008
	\file SoundManager.h
*/


#if !defined(_CNOTISOUNDMANAGER_H)
#define _CNOTISOUNDMANAGER_H

#if defined( __WIN32__ ) || defined( _WIN32 )
//
// OpenAL FrameWork
//
#include "openal/win32/Framework.h"
//
// OGG files
//
#include "vorbis/vorbisfile.h"
#else
//
// OpenAL FrameWork
//
#include "openal/MacOSX/MyOpenALSupport.h"
#endif

//
//QT
//
#include <QObject>
#include <QStringList>
//
#include <map>
//
#include "CnotiAudio.h"
#include "singleton.h"
#include "soundmanager_global.h"

namespace CnotiAudio
{
	class SoundBase;
	class Sound;
	class Music;
	class CaptureThread;
	class SourcePool;
	class NoteMisc;

	class SOUNDMANAGER_EXPORT SoundManager: public QObject, public Singleton<SoundManager>
	{
		friend class Singleton<SoundManager>;
		Q_OBJECT

	protected:
		SoundManager();
		~SoundManager();

			//static SoundManager ms_Singleton;

	public:
		void init( QString appName = "appUndefined" );

		bool release();
		bool lameMp3(const QString& filename, int minimumRate, int frequency);

		bool initOpenAl(int sourcePoolSize = 16);
		bool initOgg();

		bool copySound(const QString soundNameToCopy, const QString newSoundName);

		Sound* createSound(const QString soundName, TempoType tempo=TEMPO_160,
						   EnumInstrument instrument=FLUTE, CompassType compass=quaternario_simples);
		Music* createMusic(const QString soundName, TempoType tempo=TEMPO_160, EnumInstrument instrument=FLUTE);

		bool editRythms(const QString soundName, EnumInstrument instrument, int id, CompassType compass=quaternario_simples);
		bool loadRhythms(EnumInstrument instrument, TempoType tempo);
		QString rhythmName(EnumRhythmInstrument instrument, TempoType tempo, EnumRhythmVariation variation);
		bool loadRhythmSample(EnumRhythmInstrument instrument, TempoType tempo, EnumRhythmVariation variation);

		bool playSound(const QString soundName, bool loop=false, bool blockSignals = false);
		bool stopSound(const QString soundName);
		bool stopAllSound();
		bool pauseSound(const QString soundName);

		bool playMelodyOfSound(const QString soundName, int melody=0);

		//bool load(const QString filename, const QString name = "", bool toOverride=true);
		bool load(const QString filename, const QString name = "", bool toOverride=true, bool connectSound=true);

		bool checkSoundName(const QString soundName);

		bool releaseSound(const QString name);
		bool releaseAllSound();

		bool isSoundPlaying(const QString soundName);
		bool isSoundPaused(const QString soundName);
		bool isSoundStopped(const QString soundName);
		bool isSoundEmpty(const QString soundName);

		bool compareSound( const QString soundOne, const QString soundTwo );
		int compareMelody( const QString soundName, int firstMelody, int secondMelody );

		bool save(const QString soundName, const QString filename, bool overwrite=true);
		bool saveMelodyWav(const QString soundName, int melody, const QString filename, bool overwrite=true);
		bool saveMp3(const QString soundName, const QString filename, int minimumRate, bool deleteWav=true, bool overwrite=true);

		float percentPlay(const QString soundName);

		CnotiErrorSound getLastError();

		bool loadSampleNote(NoteType height, int octave, DurationType duration, TempoType tempo, EnumInstrument instrument);
		bool loadSamplePrincipalNotes(int octave, DurationType duration, TempoType tempo, EnumInstrument instrument);
		bool loadInstrumentSamples(EnumInstrument instrument, TempoType tempo);
		void releaseSampleNote(NoteType height, int octave, DurationType duration, TempoType tempo, EnumInstrument instrument);
		bool releaseSamplesInstrument(EnumInstrument instrument);
		bool releaseSamplesMask( QString mask );

		bool playNote(EnumInstrument instrument, TempoType tempo, DurationType duration, NoteType height, int octave = 3, float intensity=0.5);

		bool setSoundIntensity(const QString soundName, float intensity);
		bool setSoundIntensityMask( const QString mask, float intensity );
		bool setMelodyIntensity( const QString &soundName, const CnotiAudio::EnumInstrument inst, float intensity );
		float getIntensitySound(const QString soundName);
		void setIntensity( float intensity );
		Sound* getSound(const QString soundName);
		ALuint getBufferFromNote(DurationType duration, NoteType height, int octave, TempoType tempo, EnumInstrument instrument);
		short* getData(const QString soundName);
		unsigned long getSize(const QString soundName);
		ALint getFrequency(const QString soundName);
		float getDuration(const QString soundName);

		static QString nameNote(EnumInstrument instrument, TempoType tempo, DurationType duration, int octave, NoteType height);

		// Sources
		ALuint checkOutSource();
		void checkInSource( ALuint uiSource );
		bool stopSound( ALuint uiSource );

		// Capture
		void initCapture();
		void startCapture( const QString filename );
		void stopCapture();
		void endCapture();
		int getSizeCapturedBuffer();
		short* getDataCapturedBuffer();
		QByteArray* getDataCapturedBuffer( int index );
		QStringList getCaptureDeviceList();
		const QString getCaptureDevice();
		void changeCaptureDevice( const QString& deviceName );

		// Capture devices
		//QList<QString> getCaptureDevices();
		//void setCaptureDevice( const QString deviceName );

		// Log
		const QString getLogFile();

		// Note Information
		QString midiNoteToName( int midiValue );
		int midiToNote( int midiValue );
		QString getDurationTypeText( float duration, int tempo );
		int getDurationType( float duration, int tempo );
		void changeNoteMiscDeviation( float newDeviation );

		// Samples paths
		void addSamplePath(QString path);
		void addSamplePaths(QStringList paths);
		QString samplePath(QString filename);

	signals:
/*!
	This signal is emitted when a note stopped playing.
*/
		void noteStopped(QString name, int melody, int id);
/*!
	This signal is emitted when a note started to play.
*/
		void notePlaying(QString name, int melody,  int id);
/*!
	This signal is emitted when a sound is stopped.
*/
		void soundStopped(QString name);
/*!
	This signal is emitted when a sound is started to play.
*/
		void soundPlaying(QString name);
/*!
	This signal is emitted when a sound is paused.
*/
		void soundPaused(QString name);

		void signalSampleCaptured();
		void signalCaptureStopped();

	private:
		SourcePool*  _sourcePool;	// To handle source pool
		NoteMisc*    _noteMisc;		// To handle note misc functions

		typedef std::map<QString, SoundBase*>SoundList;
		SoundList _soundList;

		CnotiErrorSound _lastError;
#ifdef _WIN32
		// Ogg Voribis DLL Handle
		HINSTANCE  _g_hVorbisFileDLL;
#endif
		ALCdevice* _pDevice;

		// capture device
		ALCdevice*  _captureDevice;

		int         _hopSize;
		short*      _bigBuffer;
		short*      _hopBuffer;
		int         _bufferCount;
		int         _bufferSize;
		bool        _saveWaveFile;

		QStringList _samplesPath;

		QString     _appName;	// Name of the application using Sound Manager
		//SoundCapture*    _soundCapture; // Sound capture
		CaptureThread*   _captureThread; // Sound capture

//		void initConnect(const QString name);
		bool isInitAl;
		bool isInitOgg;
		bool isReleased;

	public:
		/**********************
		*  STATIC CONVERTERS  *
		**********************/

		static TempoType convertStrToTempo(QString tempo);
		static TempoType convertIntToTempo(int tempo);
		static EnumInstrument convertStrToInstrument(QString instrument);
		static EnumInstrument convertIntToInstrument(int instrument);
		static NoteType convertStrToHeight(QString height);
		static NoteType convertIntToHeight(int height);
		static DurationType convertStrToDuration(QString duration);
		static DurationType convertIntToDuration(int duration);
		static EnumOctave convertStrToOctave(QString octave);
		static EnumOctave convertIntToOctave(int octave);
		static EnumRhythmInstrument convertStrToRhythmInstrument(QString rhythm);
		static EnumRhythmInstrument convertIntToRhythmInstrument(int rhythm);
		static EnumRhythmVariation convertStrToRhythmVariation(QString variation);
		static EnumRhythmVariation convertIntToRhythmVariation(int variation);
	};
}

#endif  //_CNOTISOUNDMANAGER_H
