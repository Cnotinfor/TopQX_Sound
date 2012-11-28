/**
 * \file Note.cpp
 */
#include "Note.h"

namespace CnotiAudio
{
/*!
	Constructs a note in a \a position, with an \a height, an \a octave and an \a intensity.
*/
	Note::Note(DurationType duration, NoteType height, int octave)
	{
		_duration	= duration;
		_height		= height;
		_octave		= octave;
	}

/*!
	Constructs a copy of other.
*/
	Note::Note( Note &other )
	{
		this->_duration = other.getDuration();
		this->_height = other.getHeight();
		this->_octave = other.getOctave();
	}

/*!
	Destroyes the note
*/
	Note::~Note()
	{
	}
	
/*!
	Gets the note duration.
*/
	DurationType Note::getDuration()
	{
		return _duration;
	}
	
/*!
	Gets the note height.
*/
	NoteType Note::getHeight()
	{
		return _height;
	}

/*!
	Gets the note octave.
*/
	int Note::getOctave()
	{
		return _octave;
	}
	
/*!
	Returns true if other is equal to this note; otherwise returns false.

	Two notes are considered equal if they contain the same values for duration, height, octave and position.

	\sa operator!=()
*/
	const bool Note::operator==( const Note &other )
	{
		if(this->_duration == other._duration 
			&& this->_height == other._height 
			&& this->_octave == other._octave ){
			return true;
		}
		return false;
	}

/*!
	Returns true if other is not equal to this note; otherwise returns false.

	Two notes are considered equal if they contain the same values for duration, height, octave and position.
	
	\sa operator==()
*/
	const bool Note::operator!=( const Note &other )
	{
		if(this->_duration != other._duration 
			|| this->_height != other._height 
			|| this->_octave != other._octave ){
			return true;
		}
		return false;
	}

}
