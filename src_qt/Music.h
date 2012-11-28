/*!
 \class Music
 \brief The Music class contains the information of a music.

 \version 1.0
 \date 27-01-2011
 \file Melody.h
*/

#ifndef _CNOTIMUSIC_H
#define _CNOTIMUSIC_H

#include "soundBase.h"
#include "CnotiAudio.h"
#include "soundmanager_global.h"
#ifdef _WIN32
// OpenAL Framework
#include "openal/win32/aldlist.h"
#else
#include "openal/MacOSX/MyOpenALSupport.h"
#endif

namespace CnotiAudio
{
	class Note;
	class Sample;

	class SOUNDMANAGER_EXPORT Music: public SoundBase
	{
		Q_OBJECT
	public:
		Music(const QString name);

		bool load(const QString filename);
		void release();

		bool playSound(bool loop = false, bool blockSignal = false);
		bool pauseSound();
		bool stopSound();

		bool isEmpty();

		bool save(const QString filename);

		bool compareSound(SoundBase* second);
		float percentPlay();

		TempoType tempo();
		void setTempo(TempoType tempo);

		void clear();

		// Notes
		bool addNote(DurationType duration, NoteType height, int octave);
		bool deleteLastNote();
		void deleteAllNote();
		QList<Note *> noteList();
		Note *lastNote();
		int totalNotes();

		// Instruments
		EnumInstrument instrument();
		void setInstrument(EnumInstrument instrument);
		void setVolume(int instrumentId, float intensity);
		void setRhythmVolume(EnumRhythmInstrument rhythmId, float volume);

		// Rhythms
		QList<EnumRhythmInstrument> rhythmList();
		EnumRhythmVariation rhythmVariation(EnumRhythmInstrument inst);
		void addRhythm(EnumRhythmInstrument instrument, EnumRhythmVariation variation = RHYTHM_UNKNOWN, DurationType duration = SEMIBREVE);
		void changeRhythmVariation(EnumRhythmInstrument instrument, EnumRhythmVariation variation, DurationType duration = SEMIBREVE);
		bool isRhythmsOn() const;
		void setRhythmsOn(bool rhythmsOn);

		short* getData();
		unsigned long getSize();

		int durationNotes() const;

		int graphicalRepresentation() const;

	public slots:
		void saveToMp3(const QString filename, int minimumRate, bool deleteWav=true);

	signals:
		/*!	the note is stoped */
		void noteStopped(int id);
		/*!	the note is playing */
		void notePlaying(int id);
		/*!	the melody is stoped */
		void musicStopped();
		/*!	the melody is playing */
		void musicPlaying();
		/*!	the melody is paused */
		void musicPaused();
		/*! the saveToMp3Function ended */
		void saveMp3Done(bool saved);

	protected:
		typedef struct Rhythm{
			EnumRhythmInstrument instrument;
			EnumRhythmVariation  variation;
			DurationType         duration;
			float                volume;
			QString              sampleName;
		} Rhythm;

		void update();
		short* getRhythmData(Rhythm *r, int size);

	private:
		EnumInstrument  _instrument;
		int             _graphicalRepresentation; // Graphical representation of the notes
		bool            _rhythmsOn; // Variable to set if rhythm are loaded when loading the music
		TempoType       _tempo;
		QList<Note *>   _notes;	  // Note list
		QList<Rhythm *> _rhythms; // Rhythms list
		// Notes
		int	_lastNotePlay;
		int	_lastNoteStopped;
		// openAL
		ALuint _buffer[100];

		// Functions
		void fillBuffer();
		Rhythm *rhythmPtr(EnumRhythmInstrument inst);
		void removeRhythm(Rhythm *rhythm);
	};
}

#endif  //_CNOTIMUSIC_H
