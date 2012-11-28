#include "Music.h"
#include "sample.h"
#include "SoundManager.h"
#include "note.h"
// Qt
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QXmlInputSource>
#include <QDomDocument>
#include <QDomElement>
#include <QDateTime>
#include <QTextCodec>

namespace CnotiAudio
{

/*!

*/
Music::Music(const QString name) :
	SoundBase(name),
	_instrument(INSTRUMENT_UNKNOWN),
	_tempo(TEMPO_UNKNOWN),
	_graphicalRepresentation(-1),
	_rhythmsOn( true )
{

}

/*!

*/
bool Music::load(const QString filename)
{
	release();
	// Open file
	QFile *file = new QFile(filename);
	if(file->open(QIODevice::ReadOnly | QIODevice::Text))
	{
			QDomDocument doc("");
			QTextStream stream(file);
			stream.setCodec(QTextCodec::codecForName("UTF-8"));

			if (!doc.setContent(file))
			{
					qWarning() << "[Music::load] Not possible to read XML from file:" << filename;
					return false;
			}

			//Get the root element
			QDomElement root = doc.documentElement();
			if(root.tagName() != "music")
			{
					qWarning() << "[Music::load] Tag music not found:";
					return false;
			}
			_name = root.attribute("name");
			setTempo(SoundManager::convertStrToTempo(root.attribute("tempo")));
			if(_tempo == TEMPO_UNKNOWN)
			{
					qWarning() << "[Music::load] Unknown tempo";
					release();
					return false;
			}
			// Update the possible graphical represenation
			// if not set it return -1
			_graphicalRepresentation = root.attribute("representation", "-1").toInt();

			//
			// Get music information
			//
			QDomNodeList nodeList = root.elementsByTagName("melody");
			QDomElement melody = nodeList.at(0).toElement();
			setInstrument(SoundManager::convertStrToInstrument(melody.attribute("instrument")));
			if(_instrument == INSTRUMENT_UNKNOWN )
			{
					qWarning() << "[Music::load] Unknown instrument";
					release();
					return false;
			}
			//
			// Get notes
			//
			QDomNodeList musicNodeList = melody.elementsByTagName("note");
			//Check each node one by one.
			for(int i = 0;i < musicNodeList.count(); i++)
			{
					QDomElement el = musicNodeList.at(i).toElement();
					NoteType height = SoundManager::convertStrToHeight(el.attribute("height"));
					if(height == UNKNOWN_NOTE)
					{
							qWarning() << "[Music::load] Unknown note";
							release();
							return false;
					}
					DurationType duration = SoundManager::convertStrToDuration(el.attribute("duration"));
					if(duration == UNKNOWN_DURATION)
					{
							qWarning() << "[Music::load] Unknown duration";
							release();
							return false;
					}
					EnumOctave octave = SoundManager::convertStrToOctave(el.attribute("octave"));
					if(octave == OCTAVE_UNKNOWN)
					{
							qWarning() << "[Music::load] Unknown octave";
							release();
							return false;
					}
					addNote(duration, height, octave);
			}

			QDomNodeList rhythmNodeList = root.elementsByTagName("rhythm");
			//Check each node one by one.
			for(int i = 0;i < rhythmNodeList.count(); i++)
			{
					QDomElement el = rhythmNodeList.at(i).toElement();
					EnumRhythmInstrument rhythm = SoundManager::convertStrToRhythmInstrument(el.attribute("instrument"));
					if(rhythm == RHYTHM_INST_UNKNOWN)
					{
							qWarning() << "[Music::load] Unknown rhythm";
							return false;
					}
					EnumRhythmVariation rVariation = SoundManager::convertStrToRhythmVariation(el.attribute("variation"));
					if(rVariation == RHYTHM_UNKNOWN)
					{
							qWarning() << "[Music::load] Unknown rhythm variation";
							return false;
					}
					addRhythm(rhythm, rVariation);
			}

			file->close();
			_lastError = CS_NO_ERROR;
			return true;
	}
	_lastError = CS_FILE_ERROR;
	return false;

//	TODO: new way -> http://wiki.forum.nokia.com/index.php/Using_QDomDocument_to_parse_XML
}


/*!

*/
void Music::release()
{
	terminate();
	stopSound();
	clear();
}

/*!

*/
bool Music::playSound(bool loop, bool blockSignal)
{
		qDebug() << "[Music::playSound] - Loop:" << loop << "blockSignal" << blockSignal;
		_loop = loop;
		blockSignals(blockSignal);
		//
		// Get sound source
		//
		_uiSource = _soundMgr->checkOutSource();
		int error = alGetError();
		//
		// Empty
		//
		if(_notes.empty())
		{
			_lastError = CS_SOUND_EMPTY;
			qWarning() << "[Music::playSound] - Music is empty";
			return false;
		}
		//
		// Fill buffer with new information
		//
		fillBuffer();
		//
		// Queue buffer into source
		//
		alSourceQueueBuffers(_uiSource, _notes.size(), _buffer);
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
		  qWarning() << "[Melody::playSound] ERROR while queueing buffers " << error ;
		  return false;
		}
		//
		// PLAY
		//
		_lastNotePlay = 0;
		_lastNoteStopped = -1;
		alSourcePlay(_uiSource);
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
		  qWarning() << "[Music::playSound] ERROR start playing " << error;
		  return false;
		}

		if (_rhythmsOn)
		{
			Rhythm *r;
			QListIterator<Rhythm *> it(_rhythms);
			while(it.hasNext())
			{
				r = it.next();
				if(!r->sampleName.isEmpty())
				{
					_soundMgr->playSound(r->sampleName, true, true);
				}
			}
		}

		emit soundPlaying(_name);
		emit notePlaying(0);
		_stopped = false;

		// THREAD
		_flagThreadSoundStopped = false;
		start();
		return true;
}

/*!

*/
bool Music::pauseSound()
{
		return false;
}

/*!

*/
bool Music::stopSound()
{
		qDebug() << "[Music::stopSound]";
		if(isStopped() && _stopped)
		{
				_lastError = CS_IS_ALREADY_STOPPED;
				return false;
		}
		//
		// STOP
		//
		alSourceStop(_uiSource);
		//
		// update();
		//
		for(int i = _lastNoteStopped+1; i <= _lastNotePlay && i < _notes.size(); i++)
		{
				emit noteStopped(i);
		}
		int error = alGetError();
		if(error != AL_NO_ERROR)
		{
				qWarning() << "[Music::stopSound]"<< " ERROR stop playing " << error;
				return false;
		}
		// Stop rhythms
		QListIterator<Rhythm *> it(_rhythms);
		while(it.hasNext())
		{
				_soundMgr->stopSound(it.next()->sampleName);
		}
		_flagThreadSoundStopped = true;
		_stopped = true;
		qDebug() << "[Music::stopSound] Emit soundStopped";
		emit soundStopped(_name);

		_lastError = CS_NO_ERROR;
		//
		// Remove from source, buffer not played
		//
		alSourceUnqueueBuffers(_uiSource, _notes.size(), _buffer);

		_soundMgr->checkInSource(_uiSource);

		return true;
}

/*!

*/
bool Music::isEmpty()
{
		return _notes.isEmpty();
}

/*!

*/
bool Music::save(const QString filename)
{
		QString newSoundName = filename;

		if(!newSoundName.contains(".xml"))
		{
			QFileInfo fileInfo(newSoundName);
			if (fileInfo.suffix().isEmpty())
			{
				newSoundName.append(".xml");
			}
		}
		QDomDocument doc("");
		QFile file(newSoundName);
		if(file.open(QIODevice::WriteOnly))
		{
				QTextStream stream(&file);
				stream.setCodec(QTextCodec::codecForName("UTF-8"));
				stream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << "\n";

				QDomElement root = doc.createElement("music");
				doc.appendChild(root);
				root.setAttribute("name", _name);
				root.setAttribute("tempo", QString::number(_tempo));
#if QT_VERSION >= 0x070000
				root.setAttribute("date", QDateTime::currentMSecsSinceEpoch());
#endif
				//
				// Saves the music information
				//
				QDomElement melody = doc.createElement("melody");
				root.appendChild(melody);

				melody.setAttribute("instrument", _instrument);
				//
				// Saves the notes information
				//
				for(int j=0; j < _notes.size(); j++)
				{
						QDomElement note = doc.createElement("note");
						melody.appendChild(note);
						note.setAttribute("height",QString::number(_notes[j]->getHeight()));
						note.setAttribute("duration",QString::number(_notes[j]->getDuration()));
						note.setAttribute("octave",QString::number(_notes[j]->getOctave()));
				}
				//
				// Saves rhythm information
				//
				Rhythm *r;
				QListIterator<Rhythm *> it(_rhythms);
				while(it.hasNext())
				{
						r = it.next();
						if(r->variation != RHYTHM_UNKNOWN)
						{
								QDomElement rhythmElement = doc.createElement("rhythm");
								root.appendChild(rhythmElement);
								rhythmElement.setAttribute("instrument", r->instrument);
								rhythmElement.setAttribute("variation", r->variation);
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

*/
TempoType Music::tempo()
{
		return _tempo;
}

/*!

*/
void Music::setTempo(TempoType tempo)
{
		if(tempo != _tempo && tempo != TEMPO_UNKNOWN)
		{
				if(_instrument != INSTRUMENT_UNKNOWN)
				{
						_soundMgr->releaseSamplesInstrument(_instrument);
						_soundMgr->loadInstrumentSamples(_instrument, tempo);
				}
				else
				{
						qWarning() << "[Music::setTempo] Samples for instrument not loaded, instrument is unknown";
				}
				_tempo = tempo;
				// Rhythms
				Rhythm* r;
				QListIterator<Rhythm *> it(_rhythms);
				while(it.hasNext())
				{
						r = it.next();
						_soundMgr->releaseSound(r->sampleName);
						if(_soundMgr->loadRhythmSample(r->instrument, _tempo, r->variation))
						{
								r->sampleName = _soundMgr->rhythmName(r->instrument, _tempo, r->variation);
						}
				}
		}
}

/*!

*/
void Music::clear()
{
//	_tempo = TEMPO_UNKNOWN;
//	_instrument = INSTRUMENT_UNKNOWN;
		_rhythms.clear();
		deleteAllNote();
}

/*!

*/
bool Music::addNote(DurationType duration, NoteType height, int octave)
{
		Note *note = new Note(duration, height, octave);
		if(!note)
		{
				qWarning() << "[Music::addNote] Not possible to create note";
				return false;
		}
		_notes << note;
		return true;
}

/*!

*/
bool Music::deleteLastNote()
{
		if(_notes.empty())
		{
				return false;
		}
		Note * n = _notes.takeLast();
		delete(n);
		return true;
}

/*!

*/
void Music::deleteAllNote()
{
		QListIterator<Note *> it(_notes);
		while(it.hasNext())
		{
				delete(it.next());
		}
		_notes.clear();
}

/*!

*/
QList<Note *> Music::noteList()
{
		return _notes;
}

/*!

*/
Note *Music::lastNote()
{
		return _notes.last();
}

/*!

*/
int Music::totalNotes()
{
		return _notes.size();
}

/*!

*/
EnumInstrument Music::instrument()
{
		return _instrument;
}

/*!

*/
void Music::setInstrument(EnumInstrument instrument)
{
		if(instrument != _instrument && instrument != INSTRUMENT_UNKNOWN)
		{
				if(_instrument != INSTRUMENT_UNKNOWN)
				{
						_soundMgr->releaseSamplesInstrument(_instrument);
				}
				if(_tempo != TEMPO_UNKNOWN)
				{
						_soundMgr->loadInstrumentSamples(instrument, _tempo);
				}
				else
				{
						qWarning() << "[Music::setInstrument] Samples not loaded, tempo is unknown";
				}
				_instrument = instrument;
		}
}

/*!
		Sets the volume to \a volume for an intrument identified by \a instrumentId.
		Volume only changes if \a instrumentId is the current intrument.
*/
void Music::setVolume(int instrumentId, float intensity)
{
		// Check instrument
		if(instrumentId == _instrument)
		{
				_intensity = intensity;
				return;
		}
}

/*!
		Sets the volume to \a volume for a rhythm identified by \a rhythmId.
*/
void Music::setRhythmVolume(EnumRhythmInstrument rhythmId, float volume)
{
		// Check rhythms
		for(int i = 0; i < _rhythms.size(); i++)
		{
				if(rhythmId == _rhythms[i]->instrument)
				{
						_rhythms[i]->volume = volume;
						_soundMgr->setSoundIntensity(_rhythms[i]->sampleName, volume);
						break;
				}
		}
}

/*!

*/
Music::Rhythm *Music::rhythmPtr(EnumRhythmInstrument inst)
{
		Rhythm *r;
		QListIterator<Rhythm *> it(_rhythms);
		while(it.hasNext())
		{
				r = it.next();
				if(r->instrument == inst)
				{
						return r;
				}
		}
		return NULL;
}

/*!

*/
QList<EnumRhythmInstrument> Music::rhythmList()
{
		QList<EnumRhythmInstrument> rList;
		QListIterator<Rhythm *> it(_rhythms);
		while(it.hasNext())
		{
				rList << it.next()->instrument;
		}
		return rList;
}

/*!

*/
EnumRhythmVariation Music::rhythmVariation(EnumRhythmInstrument inst)
{
		Rhythm *r = rhythmPtr(inst);
		if(r == NULL)
		{
				return RHYTHM_UNKNOWN;
		}
		return r->variation;
}

/*!

*/
void Music::addRhythm(EnumRhythmInstrument instrument, EnumRhythmVariation variation, DurationType duration)
{
		qDebug() << "[Music::addRhythm] Instrument:" << instrument << "Rhythm:" << variation << "Duration" << duration;
		Rhythm *r = new Rhythm();
		r->instrument = instrument;
		r->variation = variation;
		r->duration = duration;
		if(variation != RHYTHM_UNKNOWN)
		{
				// Loads the sample for the rhythm and saves the sample name
				if(_soundMgr->loadRhythmSample(instrument, _tempo, variation))
				{
						r->sampleName = _soundMgr->rhythmName(instrument, _tempo, variation);
				}
				else
				{
						qDebug() << "[Music::addRhythm] Not possible to load rhythm sample";
				}
		}
		r->volume = _soundMgr->getIntensitySound(r->sampleName);
		_rhythms << r;
}

/*!

*/
void Music::changeRhythmVariation(EnumRhythmInstrument instrument, EnumRhythmVariation variation, DurationType duration)
{
		Rhythm *r = rhythmPtr(instrument);
		if(r != NULL)
		{
				if(variation != RHYTHM_UNKNOWN)
				{
						_soundMgr->releaseSound(r->sampleName);
						if(_soundMgr->loadRhythmSample(instrument, _tempo, variation))
						{
								r->sampleName = _soundMgr->rhythmName(instrument, _tempo, variation);
								r->variation = variation;
								return;
						}
						else
						{
								qDebug() << "[Music::changeRhythmVariation] Not possible to load rhythm sample";
						}
				}
				removeRhythm(r);
		}
		else
		{
				addRhythm(instrument, variation, duration);
		}
}

/*!

*/
void Music::update()
{
		int processed;
		alGetSourcei(_uiSource, AL_BUFFERS_PROCESSED, &processed);

		if(processed > _lastNotePlay)
		{
				//
				// some note stopped and other playing
				//
				for(int i = _lastNotePlay; i < processed; i++)
				{
						_lastNoteStopped = i;
						emit noteStopped(i);
						if(i < _notes.size() - 1)
						{
								emit notePlaying(i+1);
						}
				}
				_lastNotePlay = processed;
		}
		//
		// end of music
		//
		if(processed == _notes.size() || _notes.isEmpty())
		{
				if(_loop)
				{
						// restarts sound
						if(playSound(_loop, signalsBlocked()))
						{
								_lastError = CS_NO_ERROR;
								return;
						}
				}
				// stops sound
				stopSound();
		}
}

/*!
		Updates the data buffer to play the correct melody.
*/
void Music::fillBuffer()
{
	//
	// Fills the buffer with the data of the notes in the melody
	//
	for(int i = 0; i < _notes.size(); i++)
	{
		_buffer[i] =  _soundMgr->getBufferFromNote(
					_notes[i]->getDuration(), _notes[i]->getHeight(),
					_notes[i]->getOctave(), _tempo, _instrument
		);
	}
}

/*!

*/
short* Music::getData()
{
		if(isEmpty())
		{
				return 0;
		}

		int musicSize = getSize();
		_data = new short[musicSize];
		int notesSize = _notes.size();

		//
		// Joins the data of all the nnotes into a buffer
		//
		unsigned long index = 0;
		for(int j = 0; j < notesSize; j++)
		{
				QString noteCurrentName = _soundMgr->nameNote(_instrument, _tempo, _notes[j]->getDuration(),
																										  _notes[j]->getOctave(), _notes[j]->getHeight());
				unsigned long currentSize =_soundMgr->getSize(noteCurrentName);

				memcpy( _data+index, _soundMgr->getData(noteCurrentName), currentSize );

				index += currentSize/2.0;
		}

//	return _data;

		/**
		* mix's every rhythm to one monoral track
		*/
		Rhythm *r;
		QListIterator<Rhythm *> it(_rhythms);
		bool first = true;
		while(it.hasNext())
		{
			r = it.next();
			if(r->variation == RHYTHM_UNKNOWN)
			{
				continue; // Skip this rhythm
			}
			short *readRhythm = getRhythmData(r, musicSize); //data of the Rhythm

			float fa, fb, fresult;
			//
			// Do  mix ( integer computation for accuracy)
			//
			for( int j=0; j < musicSize; j++ )
			{
				if(first) // If is the first, start whith the melody
				{
					fa = (((_data[j] * _intensity) + 32768) / 65536.0);
				}
				else
				{
					fa = ((_data[j] + 32768) / 65536.0);
				}

				fb = (((readRhythm[j] * r->volume) + 32768) / 65536.0);
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
			if(first)
			{
				first = false;
			}
		}

		return _data;
}

unsigned long Music::getSize()
{
		unsigned long musicSize = 0;
		int notesSize = _notes.size();
		//
		// Calculates the size of the music
		//
		for(int j=0; j<notesSize; j++)
		{
				QString noteCurrentName = _soundMgr->nameNote(_instrument, _tempo, _notes[j]->getDuration(),
																										  _notes[j]->getOctave(), _notes[j]->getHeight());
				int aux = _soundMgr->getSize(noteCurrentName);
				musicSize +=  aux;
		}

		return musicSize;
}

short* Music::getRhythmData(Rhythm *r, int size)
{
		short* data = new short[size];
		// Rhythm size and data (buffer)
		unsigned long sampleSize = _soundMgr->getSize(r->sampleName);
		short* sampleData = _soundMgr->getData(r->sampleName);
		// Number of times the rhythm is repetead
		int musicRhythmRepetitions = size / sampleSize;
		// Fills data to be returned
		unsigned long index = 0;
		for(int j = 0; j < musicRhythmRepetitions; j++)
		{
				memcpy(data+index, sampleData, sampleSize);
				index += sampleSize/2.0;
		}
		// Check if still is necessary to add some more part of an rhythm
		int divisionRest = size % sampleSize;
		if( divisionRest != 0)
		{
				unsigned long sizeRemaining = size - (musicRhythmRepetitions * sampleSize);
				memcpy(data+index, sampleData, sizeRemaining);
		}
		return data;
}


bool Music::compareSound(SoundBase* second)
{
		Q_UNUSED(second);
		return false;
}

float Music::percentPlay()
{
		return 0.0f;
}

/*!
Return the duration of the notes in the music.

It is the sum of the duration of all notes
*/
int Music::durationNotes() const
{
		int duration = 0;
		QListIterator<Note*> it(_notes);

		while(it.hasNext())
		{
				switch( it.next()->getDuration() )
				{
						case CnotiAudio::SEMIBREVE:
								duration += 4;
								break;
						case CnotiAudio::MINIM_DOTTED:
								duration += 3;
								break;
						case CnotiAudio::MINIM:
								duration += 2;
								break;
						case CnotiAudio::CROTCHET:
								duration += 1;
								break;
				}
		}

		return duration;
}

/*!
	Returns the graphical represenation to use for the music. If not value is set it return -1
*/
int Music::graphicalRepresentation() const
{
	return _graphicalRepresentation;
}


void Music::saveToMp3(const QString filename, int minimumRate, bool deleteWav)
{
		emit saveMp3Done(saveMp3(filename, minimumRate, deleteWav));
}

void Music::removeRhythm(Rhythm *rhythm)
{
		_rhythms.removeOne(rhythm);
}

bool Music::isRhythmsOn() const
{
	return _rhythmsOn;
}

void Music::setRhythmsOn(bool rhythmsOn)
{
	_rhythmsOn = rhythmsOn;
}

}	// CnotiAudio


