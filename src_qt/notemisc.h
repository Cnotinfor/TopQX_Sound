#ifndef NOTEMISC_H
#define NOTEMISC_H

#include <QMap>
class QString;


namespace CnotiAudio
{
	class NoteMisc
	{
		public:
			NoteMisc();
			~NoteMisc();

		// Durations
			/*!
				Returns the name for a duration type.

				The duration type is obtain through the note \a duration and the \a tempo of the sound.
			*/
			QString getDurationTypeText( float duration, int tempo );
			/*!
				Returns the duration type through the note \a duration and the \a tempo of the sound.
			*/
			int getDurationType( float duration, int tempo );
		// MIDI
			/*!
				Returns the note name from a \a midiValue.
			*/
			QString midiNoteToName( int midiValue );
			/*!
				Returns the note value of \a midiValue.
			*/
			int midiToNote( int midiValue );
		//
		// Getters
		//
			float getDurationDeviation();
		//
		// Setters
		//
			void setDurationDeviation( float newDeviation );

		private:
			QMap<int, QString> _notesNameList;		// Keeps the names of the notes.
		// Durations
			float _durationDeviation;				// To get the interval to recognize a note.

			/*!
				Allows to load different names to the notes, name differ from country to country.

				FilePath is the path to the file, with the filename.
					C:\folder1\folder2\filename.ext

				File must be like this:
					numberNote = noteName
					ex: 512=longa
			*/
			void loadNoteInfo( QString filePath );
	};
}

#endif NOTEMISC_H
