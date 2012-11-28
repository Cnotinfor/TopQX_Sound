/*!
	\class CnotiAudio::Sound
	\brief Handles xml sound files.

	This sound is build witn one ore more melodies.

	Emits signals with the sound name and melody identification when the melody
	is started (melodyPlaying()), is paused (melodyPaused()) an is stopped
	(melodyStopped()), unless they are deactivated on playSound().

	Also emits signal with the sound name, melody identification and note
	identification when a note is started (notePlaying()) and stoped(noteStopped()).

	\sa CnotiAudio::Melody

	\version 2.0
	\data 14-11-2008
	\file Sound.h
*/


#if !defined(_CNOTISOUNDOPENAL_H)
#define _CNOTISOUNDOPENAL_H

#include "CnotiAudio.h"
#include "SoundBase.h"
#include "Melody.h"
#include "soundmanager_global.h"

namespace CnotiAudio
{
	class XmlSoundHandler;

	class SOUNDMANAGER_EXPORT Sound : public SoundBase
	{
		Q_OBJECT

	public:
		Sound( const QString name, int duration, TempoType tempo=TEMPO_160 );
		Sound( const QString name, TempoType tempo=TEMPO_160 );
		Sound( Sound &other );
		Sound( Sound &other, const QString newName );
		~Sound();

		bool load( const QString filename );
		void connectToSoundManager();

		void clear();
		void release();

		bool playSound( int melody, bool loop = false, bool blockSignal = false );
		bool playSound( bool loop, bool blockSignal );

		bool pauseSound();
		bool stopSound();

		int addMelody(EnumInstrument instrument, CompassType compass);
		int addRhythm(EnumInstrument instrument);
		bool changeRhythm( EnumInstrument instrumentz, int rythmId = 0 );
		bool changeToNextRhythm( EnumInstrument instrument );
		int rhythmVariation( EnumInstrument instrument );
		int getMelodyId( EnumInstrument instrument );

		bool addNote(DurationType duration, NoteType height, int octave = OCTAVE_C3, int intensity=127, int melody = 0);
//		bool addMultipleNote(int position, DurationType duration, NoteType height, int octave = 3, int intensity=127, int melody = 0);
		bool insertNote(int position, DurationType duration, NoteType height, int octave = 3, int intensity=127, int melody = 0);
		bool insertNoteBefore(const Note* note, DurationType duration, NoteType height, int octave = 3, int intensity=127, int melody = 0);
		bool deleteLastNote(int melodyId = 0);
		bool deleteNote(int position, NoteType height, int octave = OCTAVE_C3, int melody = 0);

		bool save(const QString filename);

		bool isPlaying();
		bool isPaused();
		bool isEmpty();

		bool compareSound(SoundBase* second);
		int compareMelody(int first_Melody, int second_melody);
		bool compareSound(Sound* second);
		float percentPlay();

		bool setInstrument(EnumInstrument instrument, int melody = 0);
		bool setMelodyTempo(TempoType tempo, int melody = 0);
		bool setTempo(TempoType tempo);
		void setSource(float x, float y, float z);
		void setIntensity(float intensity);

		int getNumberNotes(int melody=0) const;
		int getNumberMelodies() const;
		QList<Note*> getNotes(int melody=0) const;
		EnumInstrument getInstrument(int melody = 0) const;
		int getRhythmId(EnumInstrument instrument) const;
		TempoType getTempo(int melody = 0) const;
		int getTimeToStop(int melody = -1) const;
		int getDuration() const;
		int getUnitTime(int melody = 0) const;
		CnotiErrorSound getLastErrorMelody(int melody);
		QList<Melody*> getMelodyList();
		Melody* getMelody(int id);
		const int getDefaultFrequency();
		Note* getFirstNote(int melody=0);

		ALuint getBufferFromNote(DurationType duration, NoteType height, int octave, EnumInstrument instrument);
		short* getData();
		unsigned long getSize();
		short* getData(int i);
		unsigned long getSize(int i);

		// Duration selected in the creation of a new music
		void setMusicDuration( int duration );
		int getMusicDuration();

		QList<int> getGraphicBreakLines( int i = 0 );

	public slots:
		bool playSound();

	signals:
/*!
	This signal is emitted when a note stopped playing.
*/
		void noteStopped(QString name, int melody, int id);
/*!
	This signal is emitted when a note started playing.
*/
		void notePlaying(QString name, int melody,  int id);
/*!
	This signal is emitted when the melody stopped.
*/
		void melodyStopped(QString name, int melody);
/*!
	This signal is emitted when the melody start playing.
*/
		void melodyPlaying(QString name, int melody);
/*!
	This signal is emitted when the melody is paused.
*/
		void melodyPaused(QString name, int melody);

	private slots:
		virtual void noteStopped(int melody, int id);
		virtual void notePlaying(int melody, int id);
		virtual void melodyStopped(int melody);
		virtual void melodyPlaying(int melody);
		virtual void melodyPaused(int melody);

	protected:
		typedef QList<Melody*>		MelodyList;
		MelodyList							_melodyList;
		TempoType							_tempo;
		int									_musicDuration;

		// value of melody playing if is to play all the value is -1
		int									_playMelody;

		void connectMelody(int melodyId);
		void disconnectMelody(int melodyId);
		int numberOfCompassOfMelody(int melody);
		int numberOfCompass();
		bool checkIdMelody(int melodyId) const;
		bool recoverDataToHandler(XmlSoundHandler *handler);
		void update();


	};
}

#endif  //_CNOTISOUNDOPENAL_H
