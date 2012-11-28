/*!
 \class CnotiAudio::Note
 \brief The Note class oldes the information of a note.

 \version 2.1
 \date 10-11-2008
 \file Note.h
*/


#if !defined(_CNOTINOTE_H)
#define _CNOTINOTE_H

#include "CnotiAudio.h"
#include "soundmanager_global.h"

namespace CnotiAudio
{
	class SOUNDMANAGER_EXPORT Note
	{

	public:
		Note(DurationType duration, NoteType height, int octave);
		Note(Note &other );
		~Note();

		DurationType getDuration();
		NoteType getHeight();

		int getOctave();

		const bool operator==( const Note &other );
		const bool operator!=( const Note &other );

	private:
		NoteType _height;
		int _octave;
		DurationType _duration;
	};
}

#endif  //_CNOTINOTE_H
