
#include "Sound.h"
#include "Melody.h"
#include "Note.h"
#include "SoundManager.h"

#include <QDebug>

namespace CnotiAudio
{
/*!
	Constructs an empty melody.
*/
	Melody::Melody(Sound *parent, int index, EnumInstrument instrument, TempoType tempo, CompassType compass)
	{
		_index				= index;
		_instrument			= instrument;
		_compass			= compass;
		_tempo				= tempo;
		_denominatorCompass = _compass%10;
		_nominatorCompass	= _compass/10;
		_lastError			= CS_NO_ERROR;
		_unitTime			= CS_UNITTIMEDEFAULT / _denominatorCompass;
		_lastPosition		= 0;
		_totalDuration		= 0;
		_parent				= parent;
		_sourcePos[0]		= 0.0;
		_sourcePos[1]		= 0.0;
		_sourcePos[2]		= 0.0;
		_intensity			= 1.0;
		_isStopped			= true;
		_logFile            = CnotiAudio::SoundManager::instance()->getLogFile();
	}

/*!
	Constructs a copy of other.
*/
	Melody::Melody(const Melody &other)
	{
		this->_index				= other._index;
		this->_instrument			= other._instrument;
		this->_compass				= other._compass;
		this->_tempo				= other._tempo;
		this->_denominatorCompass	= other._denominatorCompass;
		this->_nominatorCompass		= other._nominatorCompass;
		this->_lastError			= other._lastError;
		this->_intensity			= other._intensity;
		this->_unitTime				= other._unitTime;
		this->_lastPosition			= other._lastPosition;
		this->_totalDuration		= other._totalDuration;
		this->_logFile		        = other._logFile;

		_parent						= other._parent;
		_sourcePos[0]				= other._sourcePos[0];
		_sourcePos[1]				= other._sourcePos[1];
		_sourcePos[2]				= other._sourcePos[2];

		for(int i=0; i<other._noteList.size(); i++){
			Note *newNote = new Note(*(other._noteList[i]));
			_noteList.push_back(newNote);
		}
	}

/*!
	Destroyes the melody.
*/
	Melody::~Melody()
	{
		//
		// Stop Melody
		//
		this->stopSound();
		//
		// Delete Notes
		//
		for(int i=0; i<_noteList.size(); i++)
			delete( _noteList[i]);
		_noteList.clear();

		_lastError = CS_NO_ERROR;
	}

/*!
	Play sound previously loaded using the \a source.

	If \a loop is false the sound will be palying once, otherwise it will keep on playing in a loop.

	returns true if sound started to play else returns false.

	\sa pauseSound() and stopSound().
*/
	bool Melody::playSound( ALuint source, bool loop, bool blockSignal )
	{
		_uiSource = source;

		blockSignals( blockSignal );

		int error = alGetError();
		int size = _noteList.size();
		if( size == 0 )
		{
			_lastError = CS_NO_ERROR;
			return true;
		}
		else
		{
//            alSourceStop( _source );
//int processed;
//alGetSourcei(_source, AL_BUFFERS_PROCESSED, &processed);
//CnotiLogManager::getSingleton().getLog(soundLog)->logMessage("Melody::playSound: " + QString::number( processed ));
			//
			// Reset source values, intensity, position, etc
			//
			resetSource();
			//
			// Fill buffer with new information
			//
			refreshBuffer();
			//
			// Queue buffer into source
			//
			alSourceQueueBuffers( _uiSource, size, _buffer);
			error = alGetError();
			if( error != AL_NO_ERROR )
			{
				qDebug() << "[Melody::playSound] ERROR while queueing buffers " << error ;
				return false;
			}
			//[
			// PLAY
			//
			_lastNotePlay = 0;
			_lastNoteStopped = -1;
			_isStopped = false;
			alSourcePlay( _uiSource );
			error = alGetError();
			if( error != AL_NO_ERROR )
			{
				qDebug() << "Melody::playSound: ERROR start playing " << error;
				return false;
			}

			emit melodyPlaying(_index);
			emit notePlaying(_index, 0);

			_lastError = CS_NO_ERROR;
			return true;
		}
	}

/*!
	Pause sound.

	If already was paused resumes playing.

	returns true if operation ended whitout errors, otherwise false.

	\sa playSound() and stopSound().
*/
	bool Melody::pauseSound()
	{
		if(!isStopped()){
			if(isPlaying()){
				//
				// PAUSE
				//
				alSourcePause( _uiSource );
				emit melodyPaused(_index);
			}
			else{
				//
				// UNPAUSE
				//
				alSourcePlay( _uiSource );
				if(alGetError() != AL_NO_ERROR){
					_lastError = CS_AL_ERROR;
					return false;
				}
				else{
					emit melodyPlaying(_index);
				}
			}
			_lastError = CS_NO_ERROR;
			return true;
		}
		else{
			_lastError = CS_IS_ALREADY_STOPPED;
			return false;
		}
	}

/*!
	Stops playing sound.

	Emits the signal noteStopped() of the note that was playing and was stopped and the signal melodyStopped().

	Returns true if operation ended without errors.

	\sa playSound() and pauseSound().
*/
	bool Melody::stopSound()
	{
		if( !isStopped() || !_isStopped )
		{
			//
			// STOP
			//
			alSourceStop( _uiSource );
			//
			// update();
			//
			for( int i = _lastNoteStopped+1; i <= _lastNotePlay; i++ )
			{
				emit noteStopped( _index, i );
			}
			int error = alGetError();
			if( error == AL_NO_ERROR )
			{
				_isStopped = true;

				_lastError = CS_NO_ERROR;
				//
				// Remove from source, buffer not played
				//
				alSourceUnqueueBuffers( _uiSource, _noteList.size(), _buffer);
				resetSource();

				emit melodyStopped(_index);
				qDebug() << "[Melody::stopSound]"<< " melodyStopped( " << _index << " )";
				_lastError = CS_NO_ERROR;
				return true;
			}
			else
			{
				qDebug() << "[Melody::stopSound]"<< " ERROR stop playing " << error;
				return true;
			}
		}
		else
		{
			_lastError = CS_IS_ALREADY_STOPPED;
			return false;
		}
	}

/*!
	Clears the object.
*/
	void Melody::clear()
	{
		_lastError			= CS_NO_ERROR;
		_lastPosition		= 0;
		_totalDuration		= 0;
		for(int i=0; i<_noteList.size(); i++)
			delete( _noteList[i]);
		_noteList.clear();
	}

/*!
	Checks if melody is empty.

	Melody is empty if it doesn't have any note.

	Return true if melody is empty;
	 */
	bool Melody::isEmpty()
	{
		return _noteList.size() == 0;
	}

/*!
	Check if melody is playing.

	Returns true if sound is playing, otherwise false.
*/
	bool Melody::isPlaying()
	{
		ALenum state;
		alGetSourcei( _uiSource, AL_SOURCE_STATE, &state );
		return state == AL_PLAYING;
	}

/*!
	Check if melody is stopped

	Returns true if melody is not playing, otherwise false.
*/
	bool Melody::isStopped()
	{
		return _isStopped;
	}

/*!
	Check if melody is paused.

	Returns true if melody is paused, otherwise false.
*/
	bool Melody::isPaused()
	{
		ALenum state;
		alGetSourcei( _uiSource, AL_SOURCE_STATE, &state );
		return state == AL_PAUSED;
	}

/*!
	Add a new note to the melody with a \a duration, an \a height, an \a octave and an \a intensity (volume).

	The note is added to the end of the melody.

	Returns true if note was added, otherwise false.
*/
	bool Melody::addNote(DurationType duration, NoteType height, int octave, int intensity)
	{
		if( !checkOctave(octave) )
		{
			_lastError = CS_VALUE_OCTAVE_ERROR;
			return false;
		}
		//
		// Adds the new note
		//
		Note *note = new Note(duration, height, octave);
		_noteList << note;
		//
		// update the total duration of sound
		//
//		if( _lastPosition <= note->getTimeToStop() )
//		{
//			_lastPosition = note->getTimeToStop();
//			_totalDuration = int( ( ( float(_lastPosition) / float(_unitTime) ) * 60000.0 ) / float(_tempo) );
//		}
		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Add a new note to the melody in the \a position with a \a duration, an \a height, an \a octave and an \a intensity (volume).

	The note is added into the position \a position of the melody. Notes after that position are moved the duration of the new note.

	Used internaly. To insert notes into a position use insertNote().

	Returns true if note was added, otherwise false.

	\sa insertNote().
*/
	bool Melody::addMultipleNote(int position, DurationType duration, NoteType height, int octave, int intensity)
	{
		if(!checkOctave(octave))
		{
			_lastError = CS_VALUE_OCTAVE_ERROR;
			return false;
		}

		Note *note = new Note(duration, height, octave);
		NoteList::iterator it=_noteList.begin();
//		for( int i=0; i < _noteList.size(); i++ )
//		{
//			if(position <= _noteList[i]->getPosition())
//				break;
//			it++;
//		}
//		_noteList.insert(it, note);
		_noteList.append(note);

		//
		// update the total duration of sound
		//
//		if(_lastPosition <= note->getTimeToStop())
//		{
//			_lastPosition = note->getTimeToStop();
//			_totalDuration = int( ( ( float(_lastPosition) / float(_unitTime) ) *60000.0 ) / float(_tempo) );
//		}

		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Add a new note to the melody before a \a note with a \a duration, an \a height, an \a octave and an \a intensity (volume).

	The note is added into the position of the \a note in the melody. The \note and other after that position are moved the duration of the new note.

	Returns true if note was added, otherwise false.
*/
	bool Melody::insertNoteBefore(const Note *note, DurationType duration, NoteType height, int octave, int intensity)
	{
		if(!checkOctave( octave ))
		{
			_lastError = CS_VALUE_OCTAVE_ERROR;
			return false;
		}

		//
		// Search note
		//
		for( int i=0; i<_noteList.size(); i++ )
		{
			if( _noteList[i] == note )
			{
				_lastError = CS_NO_ERROR;
				//
				// Insert Note
				//
				return insertNote( i/*_noteList[i]->getPosition()*/, duration, height, octave, intensity ); // TODO: needs to be fixed
			}
		}
		//
		// note don't exist
		//
		_lastError = CS_NOTE_NOT_EXISTE;
		return false;
	}

/*!
	Inserts a new note to the melody in the \a position with a \a duration, an \a height, an \a octave and an \a intensity (volume).

	The note is added into the position \a position of the melody. Notes after that position are moved one the duration of the new note.

	Returns true if note was added, otherwise false.
*/
	bool Melody::insertNote(int position, DurationType duration, NoteType height, int octave, int intensity)
	{
		if(!checkOctave(octave)){
			_lastError = CS_VALUE_OCTAVE_ERROR;
			return false;
		}
		//
		// Search given position
		//
		for(int i=0; i<_noteList.size(); i++)
		{
//			if( _noteList[i]->getPosition()>=position)		// TODO: Needs to be fixed
//			{
//				//
//				// Move notes after given position to there new position
//				//
//				int newPosition = _noteList[i]->getPosition();
//				moveNotes(newPosition, newPosition + duration);
//				//
//				// Add note to list in the given position
//				//
//				addMultipleNote(newPosition, duration, height, octave, intensity);
//				_lastError = CS_NO_ERROR;
//				return true;
//			}
		}
		// insert to the end
		addNote (duration, height, octave, intensity);
		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Moves notes in the melody from the  position \a from to the position \a to

	Returns false if no modification was made, otherwise true;
*/
	bool Melody::moveNotes(int from, int to)
	{
		bool modification = false;
		//
		// Calcs the deslocation of the notes
		//
		int distance = to - from;
		//
		// Iterates notes so see if is necessary to move it
		//
		for(int i=0; i<_noteList.size(); i++)
		{
//			if( _noteList[i]->getPosition()>=from )		// Todo: needs to be fixed
//			{
//				//
//				// Updates note position
//				//
//				_noteList[i]->setPosition( _noteList[i]->getPosition() + distance );
//				modification = true;
//			}
		}
		return modification;
	}

/*!
	Deletes the last note of the melody

	Returns true if note was deleted, otherwise false.
*/
	bool  Melody::deleteLastNote()
	{
		int size = _noteList.size();
		if(size> 0)
		{
			//
			// Deletes note & removes from list
			//
			delete(_noteList[size-1]);
			_noteList.removeLast();
			//
			// Updates the position where the melody ends
			//
			_lastPosition = 0;
//			for(int i=0; i<_noteList.size(); i++)
//			{
//				if(_lastPosition<_noteList[i]->getTimeToStop())
//				{
//					_lastPosition = _noteList[i]->getTimeToStop();
//				}
//			}
			//
			// Updates the melody total duration
			//
			_totalDuration = int( ( ( float(_lastPosition) / float(_unitTime) ) * 60000.0 ) / float(_tempo) );
			return true;
		}
		else
		{
			_lastError = CS_MELODY_EMPTY;
			return false;
		}
	}

/*!
	Deletes the note in the position \a position.

	Returns true if note was deleted, otherwise false.

NOT IMPLEMENTED
*/
	bool Melody::deleteNote(int position, NoteType height, int octave)
	{
		if(!checkOctave(octave)){
			_lastError = CS_VALUE_OCTAVE_ERROR;
			return false;
		}
//TODO: implement
		_lastError = CS_NOT_IMPLEMENT;
		return false;
	}

/*!
	Delete all notes of the melody.

	Also resets melody information.

	Returns true.
*/
	bool Melody::deleteAllNote()
	{
		//
		// Removes & deletes all the notes from the list
		//
		while(!_noteList.empty())
			_noteList.erase(_noteList.begin());
		//
		// Reste info
		//
		_lastPosition = 0;
		_totalDuration = 0;

		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Compares two melodies.

	Two melodies are equal if both contains the same notes in the same order.

	Returns -1 is melodies are not equal, otherwise the number of notes compared
*/
	int Melody::compareMelody(Melody *second)
	{
		if( second == NULL ){
			return 0;
		}
		NoteList secondNoteList = second->getNotes();
		int noteListSize = _noteList.size();
		//
		// Check sizes
		//
		if( noteListSize != secondNoteList.size() )
		{
			return -1;
		}
		//
		// Check notes
		//
		for( int i=0; i < noteListSize; i++ )
		{
			if( !(_noteList[i] == secondNoteList[i]) )
			{
				return -1;
			}
		}

		return noteListSize;
	}

/*!
	Gets melody height.

	The melody height is obtain by gettin the NoteType of the first note.

	Returns the NoteType for this melody
*/
	NoteType Melody::getHeight()
	{
		if(_noteList.size()>0)
			return _noteList[0]->getHeight();
		else
			return PAUSE;
	}

/*!
	Sets the melody intrument to \a instrument.

	Returns true.
*/
	bool Melody::setInstrument(EnumInstrument instrument)
	{
		_instrument = instrument;
		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Sets the melody tempo to \a tempo.

	Returns true.
*/
	bool Melody::setTempo(TempoType tempo)
	{
		_tempo = tempo;
		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Gets the last error that ocurred.

	Return the code for the last error.
*/
	CnotiErrorSound Melody::getLastError()
	{
		return _lastError;
	}

/*!
	Gets the melody note list.

	Returns the note list.
*/
	QList<Note*> Melody::getNotes()
	{
		return _noteList;
	}

/*!
	Gets the melody instrument.

	Returns the current instrument.
*/
	EnumInstrument Melody::getInstrument()
	{
		return _instrument;
	}

/*!
	Gets the melody tempo.

	returns the current tempo.
*/
	TempoType Melody::getTempo()
	{
		return _tempo;
	}

/*!
	Gets the melody compass.

	Returns the current compass.
*/
	CompassType Melody::getCompass()
	{
		return _compass;
	}

/*!
	Gets the melody compass numerator

	Returns the current compass numerator
*/
	int Melody::getCompass_numerator()
	{
		return _compass/10;
	}

/*!
	Gets the melody compass denominator

	Returns the current compass denominator
*/
	int Melody::getCompass_denominator()
	{
		return _compass%10;
	}

/*!
	Gets the number of notes in the melody .

	Returns the number of notes in the melody.
*/
	int Melody::getNumberNotes()
	{
		return _noteList.size();
	}

/*!
	Gets the melody duration.

	Returns the duration.
*/
	int Melody::getTotalDuration()
	{
		return _totalDuration;
	}

/*!
	Gets the melody last position.

	This position is where a new note will be appended.

	Return last position.
*/
	int Melody::getLastPosition()
	{
		return _lastPosition;
	}

/*!
	Gets the melody time unit.

	Returns the current time unit.
*/
	int Melody::getUnitTime()
	{
		return _unitTime;
	}

/*!
	Gets the Note type of the first note of the melody

	Return CnotiAudio::DO if is empty, else the NoteType of the first note.
*/
	NoteType Melody::getHeightFirstNote()
	{
		if( isEmpty() )
			return DO;
		else
			return _noteList[0]->getHeight();
	}

	Note* Melody::getFirstNote()
	{
		if( _noteList.isEmpty() )
		{
			return NULL;
		}
		else
		{
			return _noteList.first();
		}
	}

	Note* Melody::getLastNote()
	{
		if( _noteList.isEmpty() )
		{
			return NULL;
		}
		else
		{
			return _noteList.last();
		}
	}

/*!
	Verifies note octave.

	Returns true if (TODO: missing ...), otherwise false.
*/
	bool Melody::checkOctave(int octave)
	{
		if((octave>=CS_MINOCTAVE && octave<=CS_MAXOCTAVE) || octave == 0)
			return true;
		else
			return false;
	}

/*!
	Update of melody play.

	Emits the signal noteStopped() for the stopped note.
	Emits the signal notePlaying() if a new note started playing.

	If melody ended calls stopSound().

	Returns true.
*/
	bool Melody::update()
	{
		int processed;
		alGetSourcei( _uiSource, AL_BUFFERS_PROCESSED, &processed );
		//
		// If the source was delete return an error so must be quit to this function
		//
		//int error = alGetError();
		//if( error != AL_NO_ERROR )
		//{
		//	return true;
		//}

		bool toStop = false;
		if( processed > _lastNotePlay )
		{
			//
			// some note stopped and other playing
			//
			for( int i = _lastNotePlay; i < processed; i++ )
			{
				_lastNoteStopped = i;
				emit noteStopped( _index, i );
				if( i < _noteList.size() - 1 )
				{
					emit notePlaying(_index, i+1);
				}
			}
			_lastNotePlay = processed;
		}
		//
		// end of melody
		//
		if( processed == _noteList.size() || _noteList.size() == 0 )
		{
			toStop = true;
			stopSound();
		}

		_lastError = CS_NO_ERROR;
		return toStop;
	}

/*!
	Changes the intensity of the melody to \a intensity.
*/
	void Melody::setIntensity(float intensity)
	{
		_intensity = intensity;
	}

/*!
	Gets the intensity value for the melody.

	Returns the intensity.
*/
	float Melody::getIntensity()
	{
		return _intensity;
	}

/*!
	Changes the position of the sound to a new position with the values \a x, \a y and \a z.
*/
	void Melody::setSourcePosition(float x, float y, float z)
	{
		_sourcePos[0] = x;
		_sourcePos[1] = y;
		_sourcePos[2] = z;
		alSourcefv( _uiSource, AL_POSITION, _sourcePos );
	}

/*!
	Changes the sound source used to play the melody.
*/
	void Melody::setSource( ALuint uiSource )
	{
		_uiSource = uiSource;
	}

/*!
	Gets the sound source used to play the melody.

	Returns the sound source id if melody is playing, otherwise 0.
*/
	ALuint Melody::getSource()
	{
		return _uiSource;
	}

/*!
	Gets the data of the melody.

	Returns pointer to the melody data buffer.
*/
	short* Melody::getData()
	{
		unsigned long _size = getSize();
		int notesSize = _noteList.size();
		short* _data = new short[_size];

		//
		// Joins the data of all the nnotes into a buffer
		//
		unsigned long index = 0;
		for(int j=0; j<notesSize; j++){
			QString noteCurrentName = SoundManager::nameNote(_parent->getInstrument(_index), _parent->getTempo(_index),
				_noteList[j]->getDuration(), _noteList[j]->getOctave(), _noteList[j]->getHeight());

			unsigned long currentSize = SoundManager::instance()->getSize(noteCurrentName);

			memcpy( _data+index, SoundManager::instance()->getData(noteCurrentName), currentSize );

			index += currentSize/2.0;
		}

		return _data;
	}

/*!
	Gets the size of the melody.

	Returns the melody size.
*/
	unsigned long Melody::getSize()
	{
		unsigned long _size = 0;
		int notesSize = _noteList.size();
		//
		// Calculates the size of the melody
		//
		for(int j=0; j<notesSize; j++)
		{
			QString noteCurrentName = SoundManager::nameNote(_parent->getInstrument(_index), _parent->getTempo(_index),
				_noteList[j]->getDuration(), _noteList[j]->getOctave(), _noteList[j]->getHeight());
			int aux = SoundManager::instance()->getSize(noteCurrentName);
			_size +=  aux;
		}

		return _size;
	}

/*!
	Resets values of the sound source.
*/
	void Melody::resetSource()
	{
		int i = alGetError();
		ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
		ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
		ALfloat ListenerOri[] = { 0.0, 0.0, -1.0,  0.0, 1.0, 0.0 };
		//
		// Reset values
		//
		alSourcef( _uiSource, AL_GAIN, _intensity );
		alSourcefv( _uiSource, AL_POSITION, _sourcePos );
		alListenerfv( AL_POSITION,    ListenerPos );
		alListenerfv( AL_VELOCITY,    ListenerVel );
		alListenerfv( AL_ORIENTATION, ListenerOri) ;
		i = alGetError();
	}

/*!
	Updates the data buffer to play the correct melody.
*/
	void Melody::refreshBuffer()
	{
		int size = _noteList.size();
		//
		// Fills the buffer with the data of the notes in the melody
		//
		for(int i=0; i<size; i++){
			_buffer[i] = _parent->getBufferFromNote(_noteList[i]->getDuration(),
				_noteList[i]->getHeight(), _noteList[i]->getOctave(), _instrument);
		}
	}

/*!
	Set the point where in the graphical representation the music will be in another staff.
*/
	void Melody::setGraphicBreakLines( QList<int> list )
	{
		_graphicsBreakLineList = list;
	}

/*!
	Returns the break line points.
*/
	QList<int> Melody::getGraphicBreakLines()
	{
		return _graphicsBreakLineList;
	}
}
