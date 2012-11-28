#include <QString>
#include <QFile>
#include <QtXml/QDomDocument>
#include <QtXml/QXmlSimpleReader>
#include <QtXml/QXmlInputSource>
#include <QDebug>
#include <QListIterator>

#include "math.h"

#include "XmlSoundHandler.h"
#include "SoundManager.h"
#include "Sound.h"
#include "Melody.h"
#include "Note.h"
#include "LogManager.h"

#include <QDebug>

namespace CnotiAudio
{
/*!
	Constructs an empty sound with the name \a name, with the duration \a duration and with tempo \a tempo.
*/
	Sound::Sound(const QString name, int duration, TempoType tempo)
	{
		Sound( name, tempo );
		_musicDuration = duration;
		_totalDuration = 0;
	}

/*!
	Constructs an empty sound with the name \a name and with tempo \a tempo.
*/
	Sound::Sound(const QString name, TempoType tempo)
		: SoundBase(name)
	{
		_tempo			= tempo;

		// default is play all melody
		_playMelody		= -1;
		_iFrequency		= 0;

		_musicDuration	= 0;
		_totalDuration = 0;
	}

/*!
	Constructs a copy of other.
*/
	Sound::Sound(Sound &other)
		:SoundBase(other)
	{
		_tempo       = other._tempo;
		_playMelody	 = other._playMelody;
		_soundMgr    = other._soundMgr;

		_musicDuration	= other._musicDuration;

		for(int i=0; i < other._melodyList.size(); i++)
		{
			Melody *newMel = new Melody(*(other._melodyList[i]));
			//newMel->setSource(_sourcePos[0], _sourcePos[1], _sourcePos[2]);
			_melodyList.push_back(newMel);
			connectMelody(_melodyList.size()-1);
		}
		_totalDuration = 0;
	}

/*!
	Constructs a copy of other, renaming to \a newName.
*/
	Sound::Sound(Sound &other, const QString newName)
		:SoundBase(other,newName)
	{
		_tempo			= other._tempo;
		_playMelody		= other._playMelody;
		_soundMgr = other._soundMgr;

		for(int i=0; i < other._melodyList.size(); i++)
		{
			Melody *newMel = new Melody(*(other._melodyList[i]));
			//newMel->setSource(_sourcePos[0], _sourcePos[1], _sourcePos[2]);
			_melodyList.push_back(newMel);
			connectMelody(_melodyList.size()-1);
		}
		_totalDuration = 0;
	}

/*!
	Destroyes the sound.
*/
	Sound::~Sound()
	{
		release();
	}

/*!
	Releases all the data.
*/
	void Sound::release()
	{
		terminate();
		stopSound();
//		_flagThreadSoundStopped = true;
		_stopped = true;
		//alDeleteSources(1, &_uiSource);
		for( int i = 0; i < _melodyList.size(); i++ )
		{
			delete( _melodyList[i]);
		}
		_melodyList.clear();
		_lastError = CS_NO_ERROR;
	}

/*!
	Clears all the data.
*/
	void Sound::clear()
	{
		terminate();
		stopSound();
//		_flagThreadSoundStopped = true;
		_stopped = true;
		//alDeleteSources(1, &_uiSource);
		for( int i = 0; i < _melodyList.size(); i++ )
		{
			_melodyList[i]->clear();
		}
		_playMelody = -1;
		_totalDuration = 0;
		_currTime = 0;
		_pauseTime = 0;
		_lastError = CS_NO_ERROR;
	}

/*!
	Loads a sound from the XML file \a filename.

	Parses the file to get the information about the sound, melodies and notes.
*/
	bool Sound::load( const QString filename )
	{
		QFile f( filename );

		XmlSoundHandler *handler = new XmlSoundHandler();
		QXmlInputSource source( &f );
		if( source.data() == "" )
		{
			_lastError = CS_FILE_NOT_EXISTE;
			return false;
		}
		QXmlSimpleReader reader;
		reader.setContentHandler( handler );
		//
		// Retrives data from file
		//
		if( reader.parse( source ) )
		{
			//
			// Retrives data from handler
			//
			if(recoverDataToHandler(handler))
			{
				_lastError = CS_NO_ERROR;
				return true;
			}
			else
			{
				_lastError = CS_PARSER_ERROR;
			}
		}
		else
		{
			_lastError = CS_PARSER_ERROR;
		}
		return false;
	}

/*!
	Plays the sound previously loaded.

	The sound can be composed of vaious melodies, so it is possible to play all at onde (\a melodiy = -1)
	or play only one of the melodies (\a melody = melody id).

	The sound can be played in loop setting \a loop to true.

	The signals can be deactivated by setting \a blockSignal to false.

	Returns true if it was possible to play the sound, otherwise false.

	\sa loadSound(), pauseSound() and stopSound().
*/
	bool Sound::playSound( bool loop, bool blockSignal )
	{
		qDebug() << "[Sound::playSound] - Loop:" << loop << "blockSignal" << blockSignal;
		_loop = loop;
		blockSignals( blockSignal );
		int numberNotes = 0;
		//
		// Start to play all melodies
		//
		for(int i=0; i < _melodyList.size(); i++ )
		{
			if( _melodyList[i]->getNumberNotes() > 0 )
			{
				numberNotes += _melodyList[i]->getNumberNotes();
				//
				// PLAY melody
				//
				if( !_melodyList[i]->playSound( _soundMgr->checkOutSource(), loop, blockSignal ) )
				{
					_lastError = _melodyList[i]->getLastError();
					_stopped = false; // To do everything in the stopSound()
					stopSound();
					return false;
				}
			}
		}
		//
		// Empty
		//
		if( numberNotes == 0 )
		{
			_lastError = CS_SOUND_EMPTY;
			return false;
		}

		_playMelody = -1;

		qDebug() << "[Sound::playSound] - Emit soundPlaying of sound:" << _name;
		emit soundPlaying( _name );
		_currTime = 0;
		_pauseTime = 0;

		_stopped = false;
		_flagThreadSoundStopped = false;
		// THREAD
		_timer->restart();
		start();
		return true;

	}

/*!
	Plays the sound previously loaded.

	The sound can be composed of vaious melodies, so it is possible to play all at onde (\a melodiy = -1)
	or play only one of the melodies (\a melody = melody id).

	The sound can be played in loop setting \a loop to true.

	The signals can be deactivated by setting \a blockSignal to true.

	Returns true if it was possible to play the sound, otherwise false.

	\sa loadSound(), pauseSound() and stopSound().
*/
	bool Sound::playSound( int melodyId, bool loop, bool blockSignal )
	{
		qDebug() << "[Sound::playSound] - Melody ID:" << melodyId << "loop:" << "blockSignal" << blockSignal;
		if( melodyId < 0 )
		{
			//
			// play all melodies
			//
			qDebug() << "[Sound::playSound] - Play all melodies";
			return playSound( loop, blockSignal );
		}
		_loop = loop;
		blockSignals( blockSignal );
		//
		// Play only one melody
		//
		if( checkIdMelody( melodyId ) )
		{
			if( _melodyList[melodyId]->getNumberNotes() == 0)
			{
				_lastError = CS_MELODY_EMPTY;
				qWarning() << "[Sound::playSound] - melody empty";
				return false;
			}
			//
			// PLAY melody
			//
			if( !_melodyList[melodyId]->playSound( _soundMgr->checkOutSource(), loop, blockSignal ) )
			{
				_lastError = _melodyList[melodyId]->getLastError();
				qWarning() << "[Sound::playSound] - Melody:" << melodyId << "Trying to play error:" << _lastError;
				stopSound();
				return false;
			}

			_playMelody = melodyId;
		}
		else
		{
			return false;
		}

		qDebug() << "[Sound::playSound] - emit soundPlaying of sound: " << _name;
		emit soundPlaying( _name );
		_currTime = 0;
		_pauseTime = 0;

		_stopped = false;
		_flagThreadSoundStopped = false;
		// THREAD
		_timer->restart();
		start();
		return true;
	}

/*!
	Plays the sound previously loaded.

	The sound can be composed of vaious melodies, so all melodies are played.

	The sound is not played in loop.

	Returns true if it was possible to play the sound, otherwise false.
*/
	bool Sound::playSound()
	{
		qDebug() << "[Sound::playSound]";
		return playSound( false, false);
	}

/*!
	Pauses the sound.

	If it was already paused, resumes playing.

	Returns false if an error ocurred, otherwise true.

	\sa loadSound(), playSound() and stopSound().
*/
	bool Sound::pauseSound()
	{
		bool playing = isPlaying();
		if( playing || isPaused() )
		{
			if( _playMelody < 0 )
			{
				//
				// Check all melodies
				//
				for( int i=0; i < _melodyList.size(); i++ )
				{
					//
					// PAUSE / PLAY
					//
					_melodyList[i]->pauseSound();
				}
			}
			else
			{
				if( checkIdMelody( _playMelody ) )
				{
					//
					// PAUSE / PLAY
					//
					_melodyList[_playMelody]->pauseSound();
				}
				else{
					return false;
				}
			}

			if( playing )
			{
				_pauseTime += _timer->elapsed();
				qDebug() << "[Sound::pauseSound] - Emit soundPaused of sound:" << _name;
				emit soundPaused( _name );
			}
			else
			{
				_timer->restart();
				start();
				qDebug() << "[Sound::pauseSound] - Emit soundPlaying of sound:" << _name;
				emit soundPlaying( _name );
			}
			return true;
		}
		else{
			_lastError = CS_IS_ALREADY_STOPPED;
			return false;
		}
	}

/*!
	Stops the sound.

	Returns false if an error ocurred, otherwise true.

	\sa loadSound(), playSound() and pauseSound().
*/
	bool Sound::stopSound()
	{
		if( isStopped() && _stopped )
		{
			_lastError = CS_IS_ALREADY_STOPPED;
			return false;
		}
		//
		// Stop all melodies
		//
		for( int i=0; i < _melodyList.size(); i++ )
		{
			_melodyList[i]->stopSound();
			_soundMgr->checkInSource( _melodyList[i]->getSource() );
			_melodyList[i]->setSource( 0 );
		}

		_flagThreadSoundStopped = true;
		_stopped = true;
		qDebug() << "[Sound::stopSound] - Emit soundStopped of sound:" << _name;
		emit soundStopped( _name );

		return true;
	}

/*!
	Adds a melody to the sound.

	Returns the id of the melody or -1 if melody wasn�t created.
*/
	int Sound::addMelody(EnumInstrument instrument, CompassType compass)
	{
		qDebug() << "[Sound::addMelody] - Instrument:" << instrument << "Compass:" << compass;
		int numberExistingMelodies = _melodyList.size();
		//
		// Creates melody
		//
		Melody *mel = new Melody(this, numberExistingMelodies, instrument, _tempo, compass);
		if(mel != NULL)
		{
// Comment since it doesn't make sense. Disconnect should be called when deleting melodies
//			if( numberExistingMelodies == 0 )
//			{
//				// disconnect previous melody
//				disconnectMelody(numberExistingMelodies);
//			}
			//
			// Adds to list of melodies
			//
			_melodyList.push_back( mel );
			_lastError = CS_NO_ERROR;

			//
			// Connects the signals, for the first melody added to the sound.
			//
			if( numberExistingMelodies == 0 )
			{
				// connect only the first melody, used in instruments
				connectMelody(numberExistingMelodies);
				qDebug() << "[Sound::addMelody] - Melody connected:" << numberExistingMelodies;
			}

			// Sets the melody volume
			mel->setIntensity( _intensity );
			qDebug() << "[Sound::addMelody] - Melody added";

			return( numberExistingMelodies );
		}
		return -1;
	}


/*!
	Adds a rhythm to the sound.

	Returns the if for rhythm added, -1 if it already exist
*/
	int Sound::addRhythm( EnumInstrument instrument )
	{
		int melodyId = getMelodyId( instrument);

		// Test if the melody already exist for this instrument
		if( melodyId != -1)
		{
			qDebug() << "[Sound::addRhythm] - Rhythm already present.";
			return -1;
		}

		// Id for the new melody
		melodyId = _melodyList.size();

		Melody *m = new Melody(this, melodyId, instrument, _tempo, quaternario_simples);
		_melodyList.push_back( m );
		_lastError = CS_NO_ERROR;

		// Connects the signals
		//connectMelody(melodyId);

		//
		// Sets the melody volume
		//
		// TODO: each rhythm has different intensity
		//
		m->setIntensity( _intensity );
		qDebug() << "[Sound::addRhythm] - New rhythm added. ID:" << melodyId;
		return melodyId;
	}

/*!
	Changes to the next rythm, the melodies played by \a instrument.

	When reaches the maximum number of rythms for \a instrument returns to 1st rythm.

	Returns the number of instrument changed.
*/
	bool Sound::changeToNextRhythm( EnumInstrument instrument )
	{
		// TODO -> note used
		return false;
//		if( this->isEmpty() )
//		{
//			_lastError = CS_SOUND_EMPTY;
//			return false;
//		}

//		int numMelodiesChanged = 0;	// Return value;

//		int melodyId = getMelodyId( instrument );

//		if( _melodyList[melodyId]->isEmpty() )
//		{
//			// If empty changes to 1st rythm
//			return changeRhythm( instrument, melodyId, 1 );
//		}
//		else
//		{
//			// Gets current rythm
//			int rythm = _melodyList[melodyId]->getHeightFirstNote() + 1;

//			int max = 0;
//			//
//			// Max number of rythm for this instrument
//			//
//			switch( instrument )
//			{
//			case CnotiAudio::BEAT_BOX:
//					max = CS_NUMBERRYTHM_BEATBOX;
//					break;
//				default:
//					max = CS_NUMBERRYTHM;
//			}
//			//
//			// Increments rythm if smaller tham the max number of rythms, otherwise sets to first rythm.
//			//
//			( rythm < max ) ? rythm++ : rythm = 0;
//			//
//			// Changes rythm
//			//
//			if( changeRythm( instrument, melodyId, rythm ) )
//			{
//				numMelodiesChanged++;
//			}
//		}

//		return numMelodiesChanged;
	}
/*!
	Changes to the rythm with id equal to \a rythmId the melody \a melodyId played by \a instrument.

	When reaches the maximum number of rythms for \a instrument returns to 1st rythm.

	Returns true if rythm was changed, otherwise false.
*/
	bool Sound::changeRhythm( EnumInstrument instrument, int rhythmId )
	{
		qDebug() << "[Sound::changeRhythm] - Instrument:" << instrument << "Rhythm:" << rhythmId;

		// Search for a melody with the instrument
		int melodyId = getMelodyId( instrument );
		qDebug() << "[Sound::changeRythm] - Instrument:" << instrument << "melodyID:" << melodyId;
		if( rhythmId == RHYTHM_UNKNOWN )
		{
			//
			// Deletes melody
			//
			if(melodyId > -1)
			{
				_melodyList[melodyId]->clear();
				_melodyList.removeAt(melodyId);
				qDebug() << "[Sound::changeRythm] - remove rhythm";
			}
			qDebug() << "[Sound::changeRythm] - remove rhythm";
			_lastError = CS_NO_ERROR;
			return true;
		}

		if( melodyId == -1 )
		{
			// Doesn't exist so adds new rhythm
			qDebug() << "[Sound::changeRythm] - Add new rhythm";
			melodyId = addRhythm(instrument);
		}
		else
		{
			// Deletes all "notes"
			qDebug() << "[Sound::changeRythm] - clean rhythm";
			_melodyList[melodyId]->deleteAllNote();
		}

		//
		// Gets corrects values for diferent intruments
		//
		int numberCompass = numberOfCompass();
		DurationType duration;
		switch( SoundManager::convertIntToRhythmInstrument( instrument ) )
		{
			case RHYTHM_INST_BEAT_BOX:
				numberCompass = ceil( (float)numberCompass / 2 );
				duration = BREVE;
				break;
			default:
				duration = SEMIBREVE;
		}
		//
		// Adds the "notes" to the melody
		//
		for( int i = 0; i < numberCompass; i++ )
		{
			qDebug() << "[Sound::changeRythm] - Add note" << rhythmId << "duration:" << duration << "melody:" << melodyId;
			addNote( duration, (NoteType)(rhythmId), 3, 127, melodyId );
		}

		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Returns the rhythm variation
*/
	int Sound::rhythmVariation( EnumInstrument instrument )
	{
		int melodyId = getMelodyId( instrument );
		if( melodyId != -1)
		{
			return _melodyList[melodyId]->getHeightFirstNote() + 1;
		}
		return RHYTHM_UNKNOWN;
	}

/*!
	Search for a melody that uses the instrument
*/
	int Sound::getMelodyId( EnumInstrument instrument )
	{
		int i = 0;
		QListIterator<Melody*> it(_melodyList);
		while( it.hasNext() )
		{
			if( it.next()->getInstrument() == instrument )
			{
				return i;
			}
			i++;
		}

		// No melody found with instrument
		return -1;
	}

/*!
	Adds a new note to the melody

	Returns true if note was added, otherwise false.
*/
	bool Sound::addNote(DurationType duration, NoteType height, int octave, int intensity, int melody)
	{
		if( !checkIdMelody( melody ) )
		{
			qWarning() << "[Sound::addNote] - Melody:" << melody << "not found";
			return false;
		}
		//
		// Add note
		//
		if( !_melodyList[melody]->addNote( duration, height, octave, intensity ) )
		{
			_lastError = _melodyList[melody]->getLastError();
			qWarning() << "[Sound::addNote] - Trying to add note error:" << _lastError;
			return false;
		}
		//
		// Updates the sound total duration
		//
		if( _melodyList[melody]->getTotalDuration() > _totalDuration )
		{
			_totalDuration = _melodyList[melody]->getTotalDuration();
		}
		_lastError = _melodyList[melody]->getLastError();
		//
		// If necessary updates the frequency
		//
		if( _iFrequency == 0 )
		{
			_iFrequency = getDefaultFrequency();
		}

		return true;
	}

/*!
	Insert a note into \a melody in the position \a position.

	Returns ture if note was inserted, otherwsie false.
*/
	bool Sound::insertNote(int position, DurationType duration, NoteType height, int octave, int intensity, int melody)
	{
		if( !checkIdMelody( melody ) )
		{
			return false;
		}
		//
		// Inserts note.
		//
		if( !_melodyList[melody]->insertNote( position, duration, height, octave, intensity ) )
		{
			_lastError = _melodyList[melody]->getLastError();
			return false;
		}
		//
		// Updates the sound total duration
		//
		if( _melodyList[melody]->getTotalDuration() > _totalDuration )
		{
			_totalDuration = _melodyList[melody]->getTotalDuration();
		}
		_lastError = _melodyList[melody]->getLastError();
		//
		// If necessary updates the frequency
		//
		if( _iFrequency == 0 )
		{
			_iFrequency = getDefaultFrequency();
		}

		return true;
	}

/*!
	Add a new note, to the \a melody before a \a note, with a \a duration, an \a height, an \a octave and an \a intensity (volume).

	Returns true if note was added, otherwise false.

	\sa Melody::insertNoteBefore()
*/
	bool Sound::insertNoteBefore(const Note* note, DurationType duration, NoteType height, int octave, int intensity, int melody)
	{
		if( !checkIdMelody( melody ) )
		{
			return false;
		}
		//
		// Adds note.
		//
		if( !_melodyList[melody]->insertNoteBefore(note, duration, height, octave, intensity) )
		{
			_lastError = _melodyList[melody]->getLastError();
			return false;
		}
		//
		// Updates the sound total duration
		//
		if( _melodyList[melody]->getTotalDuration() > _totalDuration )
		{
			_totalDuration = _melodyList[melody]->getTotalDuration();
		}
		_lastError = _melodyList[melody]->getLastError();
		//
		// If necessary updates the frequency
		//
		if( _iFrequency == 0 )
		{
			_iFrequency = getDefaultFrequency();
		}

		return true;
	}

/*!
	Deletes the last note of the melody identified by \a melodyId.

	Returns true if note was deleted, otherwise false.
*/
	bool Sound::deleteLastNote( int melodyId )
	{
		if( checkIdMelody( melodyId ) )
		{
			//
			// Deletes last note of melodyId
			//
			bool result = _melodyList[melodyId]->deleteLastNote();
			_lastError = _melodyList[melodyId]->getLastError();
			return result;
		}

		return false;
	}

/*!
	Deletes the note in the \a position of the melody identified by \a melodyId.

	Returns true if note was deleted, otherwise false.
*/
	bool Sound::deleteNote( int position, NoteType height, int octave, int melodyId )
	{
		if( checkIdMelody( melodyId ) )
		{
			bool result = _melodyList[melodyId]->deleteNote(position, height, octave);
			_lastError = _melodyList[melodyId]->getLastError();
			return result;
		}

		return false;
	}

/*!
	Compares this sound with a \second sound

	Verifies if is it can be compared, if yes then compares.

	Returns true if the sounds are equal, otherwise false.

*/
	bool Sound::compareSound( Sound* second )
	{
		_lastError = CS_NO_ERROR;
		if( second == NULL )
		{
			return false;
		}
		//
		// Check if sound tempo & number of melodies are equal
		//
		if( _tempo != second->_tempo || _melodyList.size() != second->_melodyList.size())
		{
			return false;
		}

		for( int i=0; i < _melodyList.size(); i++ )
		{
			//
			// Checks if melodies are equal
			//
			if( _melodyList[i]->compareMelody( second->_melodyList[i] ) < 0 )
			{
				return false;
			}
			//
			// Check if instrument & compass of melodies are equal
			//
			if(_melodyList[i]->getInstrument() != second->_melodyList[i]->getInstrument()
				|| _melodyList[i]->getCompass() != second->_melodyList[i]->getCompass() )
			{
				return false;
			}
		}

		return true;
	}

/*!
	This is an overloaded member function, provided for convenience.

	Compares this sound with a \second sound

	Verifies if is it can be compared, if yes then compares.

	Returns true if the sounds are equal, otherwise false.

*/
	bool Sound::compareSound(SoundBase* second)
	{
		if( dynamic_cast<Sound*>( second ) != 0 )
		{
			return compareSound((Sound*)(second));
		}

		_lastError = CS_IS_NOT_XMLSOUND;
		return false;
	}

/*!
	Compares two melodies.

	return -1 if they are not equal, else the number of notes compared.
*/
	int Sound::compareMelody( int first_Melody, int second_melody )
	{
		if( checkIdMelody(first_Melody) && checkIdMelody(second_melody) )
		{
			int result = _melodyList[first_Melody]->compareMelody(_melodyList[second_melody]);
			_lastError = _melodyList[first_Melody]->getLastError();
			return result;
		}

		return -1;
	}

/*!
	Returns the percentage of the sound thas already was played
*/
	float Sound::percentPlay()
	{
		if( isStopped() || _totalDuration <=0 )
		{
			return 0;
		}

		if( _playMelody < 0 )
		{
			return (_currTime*1.0) / (_totalDuration*1.0);
		}
		else if( checkIdMelody( _playMelody ) )
		{
			return (_currTime*1.0) / (_melodyList[_playMelody]->getTotalDuration()*1.0);
		}

		return 0;
	}

/*!
	Saves the sound into a XML file.
*/
	bool Sound::save( const QString filename )
	{
		QString newSoundName = filename;
		if(!newSoundName.contains(".xml"))
		{
			newSoundName.append(".xml");
		}
		//filename += ".xml";
		QDomDocument doc("music [ <!ELEMENT music (melody)> <!ELEMENT melody  (note)> <!ELEMENT note  (#PCDATA)>]");
		QFile file( newSoundName );
		if( file.open( QIODevice::WriteOnly ) ) {

			QTextStream stream( &file );

			QDomElement root = doc.createElement( "music" );
			root.setAttribute( "name", _name );
			doc.appendChild(root);

			root.setAttribute( "tempo", QString::number( _tempo ) );
			if( _musicDuration != 0 )
			{
				root.setAttribute( "duration", QString::number( _musicDuration ) );
			}
			//
			// Saves the melodies information
			//
			for(int i=0; i < _melodyList.size(); i++ )
			{
				QDomElement melody = doc.createElement( "melody" );
				root.appendChild( melody );

				QList<Note*> noteList = _melodyList[i]->getNotes();
				melody.setAttribute( "instrument", _melodyList[i]->getInstrument());
				melody.setAttribute( "compass", QString::number( _melodyList[i]->getCompass()));
				//
				// Saves the notes information
				//
				for( int j=0; j < noteList.size(); j++ )
				{
					QDomElement note = doc.createElement( "note" );
					melody.appendChild( note );

					if( noteList[j]->getHeight() == PAUSE )
					{
						note.setAttribute("height",-1);
					}
					else
					{
						note.setAttribute("height",QString::number(noteList[j]->getHeight() + (noteList[j]->getOctave()+2)*CS_NUMBERNOTE));
					}
					note.setAttribute("duration",QString::number(noteList[j]->getDuration()));
//					note.setAttribute("position",QString::number(noteList[j]->getPosition()));
//					note.setAttribute("intensity",QString::number(noteList[j]->getIntensity()));
				}
			}

			doc.save(stream,4);

			file.close();
			_lastError = CS_NO_ERROR;
			return true;
		}
		_lastError = CS_FILE_ERROR;
		return false;
	}

/*!
	Returns true if sound is playing, otherwise false.
*/
	bool Sound::isPlaying()
	{
		//
		// If one melody is playing returns true
		//
		if( _playMelody < 0 )
		{
			for(int i=0; i < _melodyList.size(); i++)
			{
				if( _melodyList[i]->isPlaying() )
				{
					return true;
				}
			}
		}
		else if( checkIdMelody( _playMelody ) )
		{
			return _melodyList[_playMelody]->isPlaying();
		}

		return false;
	}

/*!
	Returns true if sound is paused, otherwise false.
*/
	bool Sound::isPaused()
	{
		if( _playMelody < 0 )
		{
			for( int i=0; i < _melodyList.size(); i++ )
			{
				if( !_melodyList[i]->isPaused() )
				{
					return false;
				}
			}
			return true;
		}
		else if( checkIdMelody( _playMelody ) )
		{
			return _melodyList[_playMelody]->isPaused();
		}

		return false;

	}
/*!
	Returns true if sound is empty, otherwise false.
*/
	bool Sound::isEmpty()
	{
		int count = 0;
		for( int i=0; i<_melodyList.size(); i++ )
		{
			count += _melodyList[i]->getNumberNotes();
		}
		_lastError = CS_NO_ERROR;
		return !count;
	}

/*!
   Changes the position \a x, \a y and \a z, where the sound will be played.
*/
	void Sound::setSource(float x, float y, float z)
	{
		_sourcePos[0] = x;
		_sourcePos[1] = y;
		_sourcePos[2] = z;
		//
		// Change for all the melodies
		//
		for( int i=0; i < _melodyList.size(); i++ )
		{
			_melodyList[i]->setSourcePosition(x, y, z);
		}
	}

/*!
   Changes the intensity value that the sound will be played.

   The new volume will be \a intensity and it will be set for all the melodies in the sound.

   The intensity value varies from 0 to 1. If \a intensity is smaller or bigger then the limit, the intensity is adjust to this values.
*/
	void Sound::setIntensity(float intensity)
	{
		if( intensity > 1.0 )
		{
			_intensity = 1.0;
		}
		else if( intensity < 0.0 )
		{
			_intensity = 0.0;
		}
		else
		{
			_intensity = intensity;
		}
		//
		// Changes for all the melodies
		//
		for( int i=0; i < _melodyList.size(); i++ )
		{
			_melodyList[i]->setIntensity(_intensity);
		}

	}

/*!
	Sets the \a melody intrument to \a instrument.

	Returns true if instrument changed, otherwise false.
*/
	bool Sound::setInstrument(EnumInstrument instrument, int melody)
	{
		qDebug() << "[Sound::setInstrument]"<< "--------------------  Sound::setInstrument(" << CnotiLogManager::number((int)instrument) <<", "<<CnotiLogManager::number(melody)<<")";
		if( checkIdMelody( melody ) )
		{
			bool result = _melodyList[melody]->setInstrument( instrument );
			_lastError = _melodyList[melody]->getLastError();
			return result;
		}

		return false;
	}
/*!
	Sets the \a melody tempo to \a tempo.

	Returns true if tempo changed, otherwise false.
*/
	bool Sound::setMelodyTempo(TempoType tempo, int melody)
	{
		qDebug() << "[Sound::setMelodyTempo]"<< "--------------------  Sound::setMelodyTempo(" << CnotiLogManager::number(tempo) <<", "<<CnotiLogManager::number(melody)<<")";
		if( checkIdMelody(melody) )
		{
			bool result = _melodyList[melody]->setTempo(tempo);
			_lastError = _melodyList[melody]->getLastError();
			return result;
		}

		return false;
	}

/*!
	Sets the sound tempo to \a tempo.

	Returns true if tempo changed, otherwise false.
*/
	bool Sound::setTempo(TempoType tempo)
	{
		qDebug() << "[Sound::setTempo]"<< "--------------------  Sound::setTempo(" << CnotiLogManager::number(tempo) <<", "<<CnotiLogManager::number(tempo)<<")";

		for( int i=0; i < _melodyList.size(); i++ )
		{
			//
			// Changes teh tempo for all the melodies
			//
			bool result = _melodyList[i]->setTempo(tempo);
			if( !result )
			{
				_lastError = _melodyList[i]->getLastError();
				return result;
			}
		}
		_tempo = tempo;
		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Returns the number of notes for the melody identified by \a melody.
*/
	int Sound::getNumberNotes(int melody) const
	{
		if( checkIdMelody(melody) )
		{
			return _melodyList[melody]->getNumberNotes();
		}
		return 0;
	}

/*!
	Returns the the number of melodies.
*/
	int Sound::getNumberMelodies() const
	{
		return _melodyList.size();
	}

/*!
	Returns the note list of the melody identified by melody.
*/
	QList<Note*> Sound::getNotes(int melody) const
	{
		if( checkIdMelody(melody) )
		{
			return _melodyList[melody]->getNotes();
		}
		//
		// Returns an empty note list
		//
		QList<Note*> empty;
		return empty;

	}

/*!
	Returns the instrument of the melody identified by melody.
*/
	EnumInstrument Sound::getInstrument(int melody) const
	{
		if( checkIdMelody(melody) )
		{
			return _melodyList[melody]->getInstrument();
		}

		return INSTRUMENT_UNKNOWN;
	}

/*!
	Returns the current rythm for \a instrument or 0 if it has no rythm.
*/
	int Sound::getRhythmId( EnumInstrument instrument ) const
	{
		if(_melodyList.size() <= 0)
		{
			//_lastError = CS_SOUND_EMPTY;
			return 0;
		}

		int i;
		for( i=0; i < _melodyList.size(); i++ )
		{
			if(_melodyList[i]->getInstrument() == instrument)
			{
				break;
			}
		}

		if( i == _melodyList.size() )
		{
			// Didn't find a melody with this instrument
			return 0;
		}
		else if( _melodyList[i]->isEmpty() )
		{
			// The melody is empty
			return 0;
		}

		// Gets rythm & returns it
		int rythmId = _melodyList[i]->getHeightFirstNote() + 1;
		return rythmId;
	}

/*!
	Returns the tempo of \a melody.
*/
	TempoType Sound::getTempo( int melody ) const
	{
		if( checkIdMelody(melody) )
		{
			return _melodyList[melody]->getTempo();
		}

		return TEMPO_UNKNOWN;
	}

/*!
	Returns the time in a staff where the \a melody will stop.
*/
	int Sound::getTimeToStop(int melody) const
	{
		if( melody < 0 )
		{
			//
			// From all the melodies
			//
			int lastPosition = 0;
			for( int i=0; i<_melodyList.size(); i++ )
			{
				if( _melodyList[i] && lastPosition < _melodyList[i]->getLastPosition() )
				{
					lastPosition = _melodyList[i]->getLastPosition();
				}
			}
			return lastPosition;
		}
		else if( checkIdMelody(melody) )
		{
			return _melodyList[melody]->getLastPosition();
		}

		return 0;
	}

/*!
	Returns the current time unit for \a melody.
*/
	int Sound::getUnitTime(int melody) const
	{
		if(checkIdMelody(melody))
			return _melodyList[melody]->getUnitTime();
		else{
			return 0;
		}
	}

/*!
	Gets the sound duration.
*/
	int Sound::getDuration() const
	{
		return _totalDuration;
	}
/*!
	Return the code for the last error in \a melody.

	If \a melody can't be found, returns the sound last error code.
*/
	CnotiErrorSound Sound::getLastErrorMelody(int melody)
	{
		if( checkIdMelody(melody) )
		{
			return _melodyList[melody]->getLastError();
		}

		return _lastError;
	}

/*!
	Retunrs the buffer from a note with a \a duration , \a height, \a ocate and \a instrument.
*/
	ALuint Sound::getBufferFromNote(DurationType duration, NoteType height,
		int octave, EnumInstrument instrument)
	{
		return _soundMgr->getBufferFromNote(duration, height, octave, _tempo, instrument);
	}

/*!
	Returns the data buffer of the sound.

	Mixes all the melodies into one buffer.
*/
	short* Sound::getData()
	{
		if(_melodyList.size()>0){

			_data = new short[getSize()];
			int auxSize = _melodyList[0]->getSize();
			memcpy(_data, _melodyList[0]->getData(), auxSize);
			long maxA, maxB = -999999;
			long minA, minB = 999999;

			/**
			* mix's every track to one monoral track
			*/
			for( int i=1; i<_melodyList.size(); i++ )
			{
				if( _melodyList[i]->isEmpty() )
				{
					continue;
				}

				int sizeMelody = (_melodyList[i]->getSize())/sizeof(short); //size of the Melody
				short *readMelody = _melodyList[i]->getData(); //data of the Melody

				int a, b, result;
				float fa, fb, fresult;
				int scaleWave = 1;
				//
				// Do  mix ( integer computation for accuracy)
				//
				for( int j=0; j < sizeMelody; j++ )
				{
					if( i == 1 )
					{
						fa = (((_data[j] * _melodyList[0]->getIntensity()) + 32768) / 65536.0);
					}
					else
					{
						fa = ((_data[j] + 32768) / 65536.0);
					}

					fb = (((readMelody[j] * _melodyList[i]->getIntensity()) + 32768) / 65536.0);
					if( fa < 0.5 || fb < 0.5 )
					{
						fresult = (((fa*fb)*2.0));
					}
					else
					{
						fresult = (2 * (fa + fb) - ((fa * fb) * 2.0) - 1);
					}

					_data[j] = (short)((fresult * 65536) - 32768);
				}
			}
		}
		else
		{
			_data = 0;
		}

		return _data;
	}

/*!
	Returns the sound size.
*/
	unsigned long Sound::getSize()
	{
		_size = 0;
		for (int i=0; i<_melodyList.size(); i++)
		{
			int aux = _melodyList[i]->getSize();
			if ( aux > _size )
				_size = aux;
		}
		return _size;
	}

/*!
	Returns the data buffer of a melody indicated by \a i.
*/
	short* Sound::getData(int i)
	{
		if( checkIdMelody(i) )
		{
			return _melodyList[i]->getData();
		}

		return 0;
	}

/*!
	Returns the size of a melody indicated by \a i.
*/
	unsigned long Sound::getSize(int i)
	{
		if( checkIdMelody(i) )
		{
			return _melodyList[i]->getSize();
		}

		return 0;
	}

/*!
	Initializes the connections of the sounds
*/
	void Sound::connectToSoundManager()
	{
		connect( this , SIGNAL(noteStopped(QString, int, int)), _soundMgr, SIGNAL(noteStopped(QString, int, int)), Qt::DirectConnection);
		connect( this , SIGNAL(notePlaying(QString, int, int)), _soundMgr, SIGNAL(notePlaying(QString, int, int)), Qt::DirectConnection);

		SoundBase::connectToSoundManager();
	}

/*!
	Makes the connections of the melody \a melodyId with this sound.
*/
	void Sound::connectMelody(int melodyId)
	{
		if(melodyId < 0 || melodyId >= _melodyList.size())
		{
			qWarning() << "[Sound::connectMelody] - Melody not found. Id:" << melodyId;
			return;
		}
		connect(_melodyList[melodyId], SIGNAL(noteStopped(int,int)), this, SLOT(noteStopped(int,int)), Qt::DirectConnection);
		connect(_melodyList[melodyId], SIGNAL(notePlaying(int,int)), this, SLOT(notePlaying(int,int)), Qt::DirectConnection);

		connect(_melodyList[melodyId], SIGNAL(melodyStopped(int)), this, SLOT(melodyStopped(int)), Qt::DirectConnection);
		connect(_melodyList[melodyId], SIGNAL(melodyPlaying(int)), this, SLOT(melodyPlaying(int)), Qt::DirectConnection);
		connect(_melodyList[melodyId], SIGNAL(melodyPaused(int)), this, SLOT(melodyPaused(int)), Qt::DirectConnection);
	}

/*!
	Unmakes the connections of the melody \a melodyId with this sound.
*/
	void Sound::disconnectMelody(int melodyId)
	{
		if(melodyId < 0 || melodyId >= _melodyList.size())
		{
			qWarning() << "[Sound::disconnectMelody] - Melody not found. Id:" << melodyId;
			return;
		}
		disconnect(_melodyList[melodyId], SIGNAL(noteStopped(int,int)), this, SLOT(noteStopped(int,int)));
		disconnect(_melodyList[melodyId], SIGNAL(notePlaying(int,int)), this, SLOT(notePlaying(int,int)));

		disconnect(_melodyList[melodyId], SIGNAL(melodyStopped(int)), this, SLOT(melodyStopped(int)));
		disconnect(_melodyList[melodyId], SIGNAL(melodyPlaying(int)), this, SLOT(melodyPlaying(int)));
		disconnect(_melodyList[melodyId], SIGNAL(melodyPaused(int)), this, SLOT(melodyPaused(int)));
	}

/*!
	Returns the number of compasses of \a melody.
*/
	int Sound::numberOfCompassOfMelody(int melody)
	{
		if(melody>=0 && melody < _melodyList.size()){
			float nbCompass = _melodyList[melody]->getLastPosition() / _melodyList[melody]->getUnitTime();
			int nbCompassInt = (int)(nbCompass);
			if(nbCompass>nbCompassInt)
				nbCompassInt++;
			return nbCompassInt;
		}
		else
			return 0;
	}

/*!
	Returns the number of compasses of the sound.
*/
	int Sound::numberOfCompass()
	{
		int lastPosition = 0;
		for( int i=0; i < _melodyList.size(); i++ )
		{
			if( lastPosition<_melodyList[i]->getLastPosition() )
			{
				lastPosition = _melodyList[i]->getLastPosition();
			}
		}

		float nbCompass = lastPosition*1.0 / CS_UNITTIMEDEFAULT*1.0;
		int nbCompassInt = (int)(nbCompass);
		if( nbCompass > nbCompassInt )
		{
			nbCompassInt++;
		}

		return nbCompassInt;
	}

/*!
	Checks if \a melodyId is a valid Id for a melody.
*/
	bool Sound::checkIdMelody( int melodyId ) const
	{
		if( melodyId >= 0 && melodyId < _melodyList.size() )
		{
			if( _melodyList[melodyId] )
			{
				//_lastError = CS_NO_ERROR;
				return true;
			}
			else
			{
				//_lastError = CS_ID_MELODY_NULL;
				return false;
			}
		}

		//_lastError = CS_ID_MELODY_UNKNOW;
		return false;
	}

//=====================================================//
//                      SLOTS                          //
//=====================================================//
/*!
	Emits noteStopped() signal.
*/
	void Sound::noteStopped(int melody, int id)
	{
		qDebug() << "[Sound::noteStopped] - Emit noteStopped of sound:" << _name << "Melody:" << melody << "Note:" << id;
		emit noteStopped( _name, melody, id);
	}
/*!
	Emits notePlaying() signal.
*/
	void Sound::notePlaying(int melody, int id)
	{
		qDebug() << "[Sound::notePlaying] - Emit notePlaying of sound:" << _name << "Melody:" << melody << "Note:" << id;
		emit notePlaying( _name, melody, id);
	}
/*!
	Emits melodyStopped() signal.
*/
	void Sound::melodyStopped(int melody)
	{
		qDebug() << "[Sound::melodyStopped] - Emit melodyStopped of sound:" << _name << "Melody:" << melody;
		emit melodyStopped( _name, melody);
	}
/*!
	Emits melodyPlaying() signal.
*/
	void Sound::melodyPlaying(int melody)
	{
		qDebug() << "[Sound::melodyPlaying] - Emit melodyPlaying of sound:" << _name << "Melody:" << melody;
		emit melodyPlaying( _name, melody);
	}
/*!
	Emits melodyPaused() signal.
*/
	void Sound::melodyPaused(int melody)
	{
		qDebug() << "[Sound::melodyPaused] - Emit melodyPaused of sound:" << _name << "Melody:" << melody;
		emit melodyPaused( _name, melody);
	}

/*!
	Updates
*/
	void Sound::update()
	{
		//THREAD
		if( _flagThreadSoundStopped )
		{
			return;
		}
		_currTime = _pauseTime + _timer->elapsed();
		//
		// All melodies are playing
		//
		if( _playMelody < 0 )
		{
			if( _melodyList.size() > 0 )
			{
				//
				// Updates Melodies
				//
				bool stopped = _melodyList[0]->update();
				for( int i=1; i < _melodyList.size(); i++ )
				{
					_melodyList[i]->update();
				}

				if( stopped )
				{
					if( _loop )
					{
						for( int i=0; i < _melodyList.size(); i++ )
						{
							//
							// Restart sound (melodies)
							//
							bool value = _melodyList[i]->playSound( _soundMgr->checkOutSource(), _loop, signalsBlocked() );
							if( !value )
							{
								_lastError = _melodyList[i]->getLastError();
								_melodyList[i]->stopSound();
								stopSound();
								return;
							}
						}
						_playMelody = -1;
						_currTime = 0;
						_pauseTime = 0;
						_timer->restart();
					}
					else
					{
						//
						// STOP
						//
						stopSound();
						return;
					}
				}
			}
		}
		//
		// Just one melody is playing
		//
		else
		{
			if( _melodyList[_playMelody]->update() )	// Check if stopped
			{
				if( _loop )
				{
					//
					// PLAY
					//
					bool value = _melodyList[_playMelody]->playSound( _soundMgr->checkOutSource(), _loop, signalsBlocked() );
					if( !value )
					{
						_lastError = _melodyList[_playMelody]->getLastError();
						_melodyList[_playMelody]->stopSound();
						stopSound();
						return;
					}
					_currTime = 0;
					_pauseTime = 0;
					_timer->restart();
				}
				else
				{
					qDebug() << "[Sound::update]"<< "emit soundStopped of sound: " << _name;

					emit soundStopped( _name );
					stopSound();
				}
			}
		}
	}

/*!
	Retrives data from xml.

	Returns true if no error ocurred, otherwise false.
*/
	bool Sound::recoverDataToHandler(XmlSoundHandler *handler)
	{
		this->release();
		_tempo = handler->getMusicTempo();
		_musicDuration = handler->getMusicDuration();
		//
		// Gets info for each melody
		//
		int nbMelody = handler->getMelodySize();
		for( int i=0; i < nbMelody; i++ )
		{
			EnumInstrument instrument = handler->getMelodyInstrument( i );
			CompassType compass = handler->getMelodyCompass( i );
			int id = addMelody(instrument, compass);
			if(id != -1)
			{
				int nbNotes = handler->getNoteListSize( i );
				_iFrequency = 0;
				//
				// Gets info for each note
				//
				for( int j=0; j < nbNotes; j++ )
				{
					DurationType duration = handler->getNoteDuration( i, j );
					int heightInt = handler->getNoteHeight( i, j );
					NoteType height;
					int octave;
					if( heightInt <= 0 )
					{
						height = PAUSE;
						octave = 3;
					}
					else
					{
						height = (NoteType)(heightInt % CS_NUMBERNOTE);
						octave = heightInt / CS_NUMBERNOTE - 2;
					}
					if( !Melody::checkOctave( octave ) )
					{
						_lastError = CS_VALUE_OCTAVE_ERROR;
						return false;
					}
					// Para já não é utilizado	//
					int intensity = handler->getNoteIntensity( i, j );
					// Para já não é utilizado	//
					int position = handler->getNotePosition( i, j );

					if(id > 0)
					{
						height = (NoteType)(height - 1);
					}
					insertNote(position, duration, height, octave, intensity, id);
				}
			}
			else
			{
				qWarning() << "[Sound::recoverDataToHandler] - Couldn�t create melody number:" << i ;
			}
			//_melodyList[i]->setGraphicBreakLines( handler->getBreakLineList() );
		}
		return true;
	}

/*!
	Returns the melody list.
*/
	QList<Melody*> Sound::getMelodyList()
	{
		return _melodyList;
	}
/*!
	Returns the pointer for a melody.
*/
	Melody* Sound::getMelody(int id)
	{
		// If value between 0 and (number of melodies - 1) including this values
		if(id < 0 || _melodyList.count() < id )
		{
			return NULL;
		}
		return _melodyList.at(id);
	}

/*!
	Changes the music duration
*/
	void Sound::setMusicDuration( int duration )
	{
		_musicDuration = duration;
	}

/*!
	Returns the music duration.
*/
	int Sound::getMusicDuration()
	{
		return _musicDuration;
	}

/*!
	Returns the default frequency.
*/
	const int Sound::getDefaultFrequency()
	{
		return 22050;
	}

/*!
	Returns the first note of a melody. If the melody is empty or the melody doesn't exist it returns
	NULL.
*/
	Note* Sound::getFirstNote(int melody)
	{
		// Check if it is a valid melody
		if( _melodyList.size() <= melody)
		{
			return NULL;
		}

		return _melodyList[melody]->getFirstNote();
	}

	QList<int> Sound::getGraphicBreakLines( int i /*= 0 */ )
	{
		QList<int> list;
		if( i < _melodyList.size() )
		{
			list = _melodyList[i]->getGraphicBreakLines();
		}
		return list;
	}
}

