/*!
 \class CnotiAudio::Melody
 \brief The Melody class oldes the information of a melody.

 \version 2.1
 \date 10-11-2008
 \file Melody.h
*/

#if !defined(_CNOTIMELODY_H)
#define _CNOTIMELODY_H

#include <QObject>
//#include <vector>
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
	class Sound;
	class Note;

	class SOUNDMANAGER_EXPORT Melody: public QObject
	{
		Q_OBJECT

	public:
		Melody(Sound *parent, int index, EnumInstrument instrument, TempoType tempo, CompassType compass);
		Melody(const Melody &other);
		~Melody();

		bool playSound(ALuint source, bool loop = false, bool blockSignal = false );
		bool pauseSound();
		bool stopSound();

		void clear();

		bool addNote(DurationType duration, NoteType height, int octave, int intensity);
		bool insertNoteBefore(const Note *note, DurationType duration, NoteType height, int octave, int intensity);
		bool insertNote(int position, DurationType duration, NoteType height, int octave, int intensity);
		bool moveNotes(int from, int to);
		bool deleteLastNote();
		bool deleteNote(int position, NoteType height, int octave);
		bool deleteAllNote();

		int compareMelody(Melody *second);

		NoteType getHeight();

		bool isEmpty();
		bool isPlaying();
		bool isStopped();
		bool isPaused();

		bool update();

		bool setInstrument(EnumInstrument instrument);
		bool setTempo(TempoType tempo);

		CnotiErrorSound getLastError();
		EnumInstrument getInstrument();
		TempoType getTempo();
		CompassType getCompass();
		int getCompass_numerator();
		int getCompass_denominator();
		int getNumberNotes();

		QList<Note*> getNotes();
		int getTotalDuration();
		int getLastPosition();
		int getUnitTime();
		NoteType getHeightFirstNote();
		Note* getFirstNote();
		Note* getLastNote();

		float getIntensity();
		void setIntensity(float intensity);
		void setSourcePosition(float x, float y, float z);
		void setSource( ALuint uiSource );
		ALuint getSource();

		short* getData();
		unsigned long getSize();

		void setGraphicBreakLines( QList<int> list );
		QList<int> getGraphicBreakLines();

		static bool checkOctave(int octave);

	signals:
		/*!	the note is stoped */
		void noteStopped(int melody, int id);
		/*!	the note is playing */
		void notePlaying(int melody,  int id);
		/*!	the melody is stoped */
		void melodyStopped(int melody);
		/*!	the melody is playing */
		void melodyPlaying(int melody);
		/*!	the melody is paused */
		void melodyPaused(int melody);

	protected:
		typedef QList<Note*> NoteList;

		NoteList			_noteList;
		int					_index;
		float				_intensity;
		EnumInstrument		_instrument;
		TempoType			_tempo;
		CompassType			_compass;
		int					_denominatorCompass;
		int					_nominatorCompass;
		CnotiErrorSound		_lastError;

		int					_lastPosition;
		int					_totalDuration;
		int					_unitTime;

		Sound*              _parent;
		ALuint				_buffer[100];
		ALuint				_uiSource;
		ALfloat				_sourcePos[3];

		int					_lastNotePlay;
		int					_lastNoteStopped;
		bool				_isStopped;

		QList<int> _graphicsBreakLineList;

	protected:	// Functions
		void resetSource();
		void refreshBuffer();
		bool addMultipleNote(int position, DurationType duration, NoteType height, int octave, int intensity);

	private:
		QString         _logFile;
	};
}

#endif  //_CNOTIMELODY_H
