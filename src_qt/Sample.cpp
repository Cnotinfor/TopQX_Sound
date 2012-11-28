#include <QString>
#include <QFile>

#include "SoundManager.h"
#include "Sample.h"
#include "LogManager.h"

#include <QDebug>

namespace CnotiAudio
{
/*!
	Constructs an empty sample with the name \a name.
*/
	Sample::Sample( const QString name )
		: SoundBase( name )
	{
		_buffer		= 0;
		_data		= 0;
		_size		= 0;
	}

/*!
	Constructs a copy of other.
*/
	Sample::Sample( Sample &other )
		: SoundBase( other )
	{
		_buffer			= other._buffer;
		_size			= other._size;
	}

/*!
	Constructs a copy of other, renaming to \a newName.
*/
	Sample::Sample( Sample &other, const QString newName )
		: SoundBase( other, newName )
	{
		_buffer			= other._buffer;
		_size			= other._size;
	}
/*!
	Destroyes the sample.
*/
	Sample::~Sample()
	{
		release();
	}

/*!
	Releases all the data.
*/
	void Sample::release()
	{
		terminate();
		stopSound();
//		_flagThreadSoundStopped = false;
		//
		// Delete buffer
		//
		alDeleteBuffers(1 , &_buffer);

		_dataMutex.lock();
//TODO: check why sometimes it gives error here
		//if(_data)
		//{
		//	delete(_data);
		//}
		_dataMutex.unlock();
		//
		// Resets data
		//
		_size = 0;
		_buffer = 0;
//		_iFrequency = 0;
		_lastError = CS_NO_ERROR;
	}

/*!
	Loads a sample from the file \a filename.
*/
	bool Sample::load( const QString filename )
	{
		alGetError();
		//
		// Removes the previous buffers and generates a new one
		//
		alDeleteBuffers( 1, &_buffer );
		alGetError();
		alGenBuffers( 1, &_buffer );
		if( alGetError() != AL_NO_ERROR )
		{
			qDebug() << "[Sample::loadWav]"<< " Error: While creating AL buffer ";
			return false;
		}

#if defined(__WIN32__) || defined(_WIN32) || defined(Q_WS_WIN) || defined(Q_WS_WIN32)
		int				WaveID;
		ALint			iDataSize;
		ALenum			eBufferFormat;
		ALchar			*pData;
		WAVEID			wId;
		//
		// Loads wav file
		//
		CWaves waves;
		if( waves.LoadWaveFile( filename.toStdString().c_str(), &wId ) != WR_OK )
		{
			return false;
		}
		int i = alGetError();
		//
		// Tries to gets data from file
		//
		if ((SUCCEEDED(waves.GetWaveSize(wId, (unsigned long*)&iDataSize))) &&
			(SUCCEEDED(waves.GetWaveData(wId, (void**)&pData))) &&
			(SUCCEEDED(waves.GetWaveFrequency(wId, (unsigned long*)&_iFrequency))) &&
			(SUCCEEDED(waves.GetWaveALBufferFormat(wId, &alGetEnumValue, (unsigned long*)&eBufferFormat))))
		{

			i = alGetError();
			//
			// Set XRAM Mode (if application)
			//
			if (eaxSetBufferMode && 0)
				eaxSetBufferMode(1, &_buffer, 0);

			i = alGetError();
			//
			// Gets data from file to buffer
			//
			alBufferData(_buffer, eBufferFormat, pData, iDataSize, _iFrequency);
			if (alGetError() == AL_NO_ERROR)
				_lastError = CS_NO_ERROR;
			else
			{
				//CnotiLogManager::getSingleton().getLog(_logFile)->logMessage("[Sample::loadWav] Error: Copying wave data to AL Buffer");
				qDebug() << "[Sample::loadWav] " << "Error: Copying wave data to AL Buffer";
			}
			//
			// Copies data
			//
			_dataMutex.lock();
			_data = new short[iDataSize];
			memcpy(_data, (short*)pData, iDataSize);
			_dataMutex.unlock();
			_size = iDataSize;
			//
			// Releases wave file
			//
			waves.DeleteWaveFile(wId);
		}

#else
                ALenum  format;
                ALvoid* data;
                ALsizei size;
                ALsizei freq;
                //
                // get some audio data from a wave file
                //
                CFStringRef fileName = CFStringCreateWithCString( NULL, filename.toStdString().c_str(), kCFStringEncodingUTF8 );
                CFStringRef fileNameEscaped = CFURLCreateStringByAddingPercentEscapes(NULL, fileName, NULL, NULL, kCFStringEncodingUTF8);
                CFURLRef fileURL = CFURLCreateWithString(kCFAllocatorDefault, fileNameEscaped, NULL);
                data = MyGetOpenALAudioData(fileURL, &size, &format, &freq);
                CFRelease(fileURL);

				if(data == NULL)
				{
					qDebug() << "[Sample::loadWav] " << "Error: loading file" << filename;
					return false;
                }
                //
                // Copy data
                //
                _dataMutex.lock();
                _data = new short[size];
                memcpy(_data, (short*)data, size);
                _dataMutex.unlock();
                _size = size;
                //
                // Attach Audio Data to OpenAL Buffer
                //
                alBufferData(_buffer, format, data, size, freq);
                //
                // Release the audio data
                //
                free(data);

                if(alGetError() != AL_NO_ERROR)
				{
					qDebug() << "Sample::loadWav] "<< "Error: Copying wave data to AL Buffer";
                }
                else
				{
					_lastError = CS_NO_ERROR;
				}
#endif

		if(_lastError == CS_NO_ERROR)
			return true;
		else
			return false;
	}

/*!
	Plays the sample previously loaded.

	The sample can be played in loop setting \a loop to true.

	The signals can be deactivated by setting \a blockSignal to false.

	Returns true if it was possible to play the sample, otherwise false.

	\sa loadSound(), pauseSound() and stopSound().
*/
	bool Sample::playSound( bool loop, bool blockSignal )
	{
		_loop = loop;
		blockSignals( blockSignal );

		ALenum error = alGetError();

		// Gets a new source
		_uiSource = _soundMgr->checkOutSource();

		if( isPlaying() )
		{
			stopSound();
		}

		// Set intensity
		alGetError(); // clean previous errors
		alSourcef( _uiSource, AL_GAIN, _intensity );
		error = alGetError();
		if( error != AL_NO_ERROR )
		{
			qWarning() << "[Sample::playSound] Not possible to change the Intensity to:" << _intensity << alGetString( error );
		}

		// Set buffer to be played
		alSourcei( _uiSource, AL_BUFFER, _buffer );
		if( error = alGetError() != AL_NO_ERROR )
		{
			qDebug() << "[Sample::playSound] " << "Error: Associate a Buffer to a Source";
		}

		//
		// Reset
		//
		_currTime = 0;
		_pauseTime = 0;

		//
		// Play
		//
		alSourcePlay( _uiSource );
		if(alGetError() != AL_NO_ERROR)
		{
			qDebug() << "[Sample::playSound]" << " Error: CS_AL_ERROR - not possible to alSourcePlay";

			stopSound();
			_lastError = CS_AL_ERROR;
			return false;
		}
		_flagThreadSoundStopped = false;
		_stopped = false;
		emit soundPlaying( _name );
		if(!signalsBlocked())
		{
			qDebug() << "[Sample::playSound]"<< " emit soundPlaying of sound: "<< _name;
		}

		//
		// If is loop and signals are bloock we can use the loop in OpenAL.
		// With this we don't need to start the thread
		//
		if( _loop && signalsBlocked() )
		{
			alSourcei( _uiSource, AL_LOOPING, AL_TRUE );
			return true;
		}

		//
		// THREAD
		//
		_timer->restart();
		start();			// Necessary to realease sound source

		return true;

	}

/*!
	Pauses the sample.

	If it was already paused, resumes playing.

	Returns false if an error ocurred, otherwise true.

	\sa loadSound(), playSound() and stopSound().
*/
	bool Sample::pauseSound()
	{
		if( isPlaying() )
		{
			//
			// PAUSE
			//
			alSourcePause( _uiSource );
			_pauseTime += _timer->elapsed();

			if(!signalsBlocked())
			{
				qDebug() << "[Sample::pauseSound]" << " emit soundPaused of sound: "<< _name;
			}
			emit soundPaused( _name );
			_lastError = CS_NO_ERROR;
			return true;
		}
		else if( isPaused() )
		{
			//
			// RESUME PLAY
			//
			alSourcePlay( _uiSource );
			if(alGetError() == AL_NO_ERROR)
			{
				_timer->restart();
				start();
				if(!signalsBlocked())
				{
					qDebug() << "[Sample::pauseSound]" << " emit soundPlaying of sound: "<< _name;
				}
				emit soundPlaying( _name );
				_lastError = CS_NO_ERROR;
				return true;
			}
			else
			{
				_lastError = CS_AL_ERROR;
				return false;
			}
		}
		else
		{
			_lastError = CS_IS_ALREADY_STOPPED;
			return false;
		}
	}

/*!
	Stops the sample.

	Returns false if an error ocurred, otherwise true.

	\sa loadSound(), playSound() and pauseSound().
*/
	bool Sample::stopSound()
	{
		if( _loop && signalsBlocked() )
		{
			//
			// Disables looping for this source
			//
			if( _uiSource )
			{
				alSourcei( _uiSource, AL_LOOPING, AL_FALSE );
			}
		}

		if( !_stopped )
		{
			//
			// STOP
			//
			if( _uiSource )
			{
				if( !isStopped() )
				{
					int error = alGetError(); // clear last error
					// Stop source
					alSourceStop( _uiSource );
					error = alGetError();
					if( error != AL_NO_ERROR )
					{
						qWarning() << "[Sample::stopSound] ERROR stop playing " << error;
						_soundMgr->checkInSource( _uiSource );
						return false;
					}
				}
				_soundMgr->checkInSource( _uiSource );
				_uiSource = 0;
			}
			_flagThreadSoundStopped = true;
			_stopped = true;
			if(!signalsBlocked())
			{
				qDebug() << "[Sample::stopSound()]"<< " emit soundStopped of sound: "<< _name;
			}
			emit soundStopped( _name );
			return true;
		}
		else
		{
			_lastError = CS_IS_ALREADY_STOPPED;
			return false;
		}
	}

/*!
	Returns true if sample is empty, otherwise false.
*/
	bool Sample::isEmpty()
	{
		return _data == 0;
	}

/*!
	Returns the sample buffer.
*/
	ALuint Sample::getBuffer()
	{
		return _buffer;
	}

/*!
	Compares two samples.

	Two samples are equal if ...

	Returns true if samples are equal, otherwise false.

	NOT YET IMPLEMENTED
*/
	bool Sample::compareSound(SoundBase* second)
	{
		if( dynamic_cast<Sample*>(second) == 0 )
		{
			_lastError = CS_IS_NOT_SAMPLE;
			return false;
		}
		else
		{
			_lastError = CS_NOT_IMPLEMENT;
			return false;
		}
	}

/*!
	Returns percent play of the sample.
*/
	float Sample::percentPlay()
	{
		return (_currTime*1.0) / (_totalDuration*1.0);
	}

/*!
	Saves a sample.

	Returns true if sounds was, otherwise false.

	NOT YET IMPLEMENTED
*/
	bool Sample::save( const QString filename )
	{
		_lastError = CS_NOT_IMPLEMENT;
		return false;
	}

/*!
	Verifies if the music ended.
*/
	void Sample::update()
	{
		if( isStopped() )
		{
//            if( !_flagThreadSoundStopped ) { // Check if was not manually stopped previously
			qDebug() << "[Sample::update()]"<< " --------- Going to stop sound: "<< _name;
			stopSound();
//            }
		}
	}
}
