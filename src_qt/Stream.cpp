/**
	\file Stream.cpp
*/
#include "Stream.h"

//
// Qt
//
#include <QString>

#include <QDebug>

#include "SoundManager.h"
#include "LogManager.h"

//
// OGG 
//
LPOVCLEAR					fn_ov_clear;
LPOVREAD					fn_ov_read;
LPOVPCMTOTAL				fn_ov_pcm_total;
LPOVINFO					fn_ov_info;
LPOVCOMMENT					fn_ov_comment;
LPOVOPENCALLBACKS			fn_ov_open_callbacks;

size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
int ov_seek_func(void *datasource, ogg_int64_t offset, int whence);
int ov_close_func(void *datasource);
long ov_tell_func(void *datasource);
void Swap(short &s1, short &s2);

namespace CnotiAudio
{
/*!
	Constructs an empty stream with the name \a name.
*/
	Stream::Stream( const QString name )
		: SoundBase( name )
	{
		_sCallbacks.read_func = ov_read_func;
		_sCallbacks.seek_func = ov_seek_func;
		_sCallbacks.close_func = ov_close_func;
		_sCallbacks.tell_func = ov_tell_func;

		_data			= 0;
		_size			= 0;
		_ulFrequency	= 0;
		_ulBufferSize	= 0;
		_ulChannels		= 0;
		_ulFormat		= 0;

        _uiSource       = 0;

		_filename		= "";
		_streamingStarted	= false;
		
		_pDecodeBuffer	= NULL;
		_psVorbisInfo	= NULL;
		_sOggVorbisFile	= new OggVorbis_File;
	}

	/*!
	Constructs a copy of other.
*/
	Stream::Stream( Stream &other )
		: SoundBase( other )
	{
		_sCallbacks.read_func = ov_read_func;
		_sCallbacks.seek_func = ov_seek_func;
		_sCallbacks.close_func = ov_close_func;
		_sCallbacks.tell_func = ov_tell_func;

		_data			= 0;
		_size			= 0;
		_ulFrequency	= 0;
		_ulBufferSize	= 0;
		_ulChannels		= 0;
		_ulFormat		= 0;

        _uiSource       = 0;

		_filename		= other._filename;
		_streamingStarted = false;
		
		_pDecodeBuffer	= NULL;
		_psVorbisInfo	= NULL;
	}

/*!
	Constructs a copy of other, renaming to \a newName.
*/
	Stream::Stream( Stream &other, const QString newName )
		: SoundBase( other, newName )
	{
		_sCallbacks.read_func = ov_read_func;
		_sCallbacks.seek_func = ov_seek_func;
		_sCallbacks.close_func = ov_close_func;
		_sCallbacks.tell_func = ov_tell_func;

		_data			= 0;
		_size			= 0;
		_ulFrequency	= 0;
		_ulBufferSize	= 0;
		_ulChannels		= 0;
		_ulFormat		= 0;

        _uiSource       = 0;

		_filename		= other._filename;
		_streamingStarted = false;
		
		_pDecodeBuffer	= NULL;
		_psVorbisInfo	= NULL;
	}

/*!
	Destroyes the stream.
*/
	Stream::~Stream()
	{
		if( isPlaying() )
		{
			stopSound();
		}

		release();

		alDeleteBuffers( NUMBUFFERSOGG, _uiBuffers );

		if( _pDecodeBuffer )
		{
			free( _pDecodeBuffer );
			_pDecodeBuffer = NULL;
		}

		//fn_ov_clear(sOggVorbisFile);
		if( _sOggVorbisFile )
		{
			delete( _sOggVorbisFile );
		}

	}

/*!
	Saves the name of file.

	Ogg is played in stream, so the load is made while playing.
*/
	bool Stream::load(const QString filename)
	{
		_filename = filename;
		return true;
	}
	
	void Stream::release()
	{			
		_lastError = CS_NO_ERROR;
	}
	
/*!
	Starts playing the stream of file given in loadSound( const QString& filename ).

	The stream can be played in loop setting \a loop to true.

	The signals can be deactivated by setting \a signal to false.

	Returns true if it was possible to play the sample, otherwise false.

	\sa loadSound(), pauseSound() and stopSound().
*/
	bool Stream::playSound( bool loop, bool blockSignal )
	{		
		_loop = loop;
		blockSignals( blockSignal );
		
		qDebug() << "[Stream::playSound]"<< "------------  Stream::playSound( File: " << _filename << " Loop: " << CnotiLogManager::boolean(loop) << ")";
		
		int error = alGetError();

		if( isPlaying() )
		{
			qDebug() << "[Stream::playSound]"<< "----------------------------- ERROR sound stream is too playing... Restarting";
			
			_lastError = CS_IS_ALREADY_PLAYING;
            this->stopSound();
		}

		//
		// Reset some values
		//
		_currTime = 0;
		_pauseTime = 0;

		//
		// Gets a new source & set source volume
		//
		_uiSource = _soundMgr->checkOutSource();
		alSourcef( _uiSource, AL_GAIN, _intensity);
		
		if( !_streamingStarted )
		{
			//
			// Start Streaming
			//
			if( !startStreaming() )
			{
				alDeleteBuffers( NUMBUFFERSOGG, _uiBuffers );
				_streamingStarted = false;
				qDebug() << "[Stream::playSound]" << " ERROR streaming start failed";
				return false;
			}
		}
		else
		{
			qDebug() << "[Stream::playSound]" << "  ----------------------------- streaming already started";
		}
		//
		// PLAY 
		//
        qDebug() << "[Stream::playSound]" << "  ----------------------------- play source";
		_flagThreadSoundStopped = false;
        alSourcePlay( _uiSource );
		error = alGetError();
	    if( error != AL_NO_ERROR )
		{
            qDebug() << "[Stream::playSound]" << "  ----------------------------- ERROR openal failed";
		    alDeleteBuffers( NUMBUFFERSOGG, _uiBuffers );
		    stopSound();
			_lastError = CS_AL_ERROR;
		    return false;
        }
		emit soundPlaying( _name );
        qDebug() << "[Stream::playSound]" << "  ----------------------------- emit soundPlaying of sound: " << _name;
		//
		// THREAD
		//
		_timer->restart();
		start();
		qDebug() << "[Stream::playSound]" << "  ----------------------------- thread start";

		return true;
	}

/*!
	Pauses the sample.

	Returns false if an error ocurred, otherwise true.

	\sa loadSound(), playSound() and pauseSound().

	NOT IMPLEMENTED YET
*/
	bool Stream::pauseSound()
	{
		_lastError = CS_NOT_IMPLEMENT;
		return false;
	}
	
/*!
	Stops the stream.

	Returns false if an error ocurred, otherwise true.

	\sa loadSound(), playSound() and pauseSound().
*/
	bool Stream::stopSound()
	{
		bool result = false;

        _mutex.lock();
        if( !_flagThreadSoundStopped )
		{
			//
			// STOP
			//
			if( _uiSource )
			{
				if( !isStopped() )
				{
					alSourceStop( _uiSource );
				}
				_soundMgr->checkInSource( _uiSource );
				_uiSource = 0;
			}

		    _flagThreadSoundStopped = true;
		    qDebug() << "[Stream::playSound]"<< " --------- emit soundStopped of sound: "<< _name;
		    emit soundStopped( _name );

		    if( _pDecodeBuffer )
			{
			    free( _pDecodeBuffer );
			    _pDecodeBuffer = NULL;
		    }
		    qDebug() << "[Stream::stopSound]" << " _pDecodeBuffer release";
		    if( _sOggVorbisFile )
			{
			    fn_ov_clear( _sOggVorbisFile );
			}
		    qDebug() << "[Stream::stopSound]" << " sOggVorbisFile clear";
		    
			_streamingStarted = false;
			_iTotalBuffersProcessed = 0;
        
			result = true;
		}
		else{
			_lastError = CS_IS_ALREADY_STOPPED;
            
			 result = false;
		}

        _mutex.unlock();
		return result;
	}
	
/*!
	Start stream of file.

	Gets information about the file (Channels, Format, and Frequency) 
	and then decodes the audio data from the OggVorbis file to fill the buffers.
*/
	bool Stream::startStreaming()
	{	
		if( _streamingStarted )
		{
			return false;
		}
		_streamingStarted = true;
		//
		// Open file & generate ogg buffer
		//
		FILE *pOggVorbisFile = fopen(_filename.toStdString().c_str(), "rb");
		alGenBuffers( NUMBUFFERSOGG, _uiBuffers );

		if( fn_ov_open_callbacks( pOggVorbisFile, _sOggVorbisFile, NULL, 0, _sCallbacks ) != 0 ){
            
            qDebug() << "[Stream::startStreaming()]"<< " --------- ERROR: fn_ov_open_callbacks failed --- FILE: "<< _filename;
            
			_lastError = CS_ERROR_FILE_OGG;
			return false;
		}
		int error = alGetError();
		// Gets some information about the file (Channels, Format, and Frequency)
		_psVorbisInfo = fn_ov_info( _sOggVorbisFile, -1 );
		if( _psVorbisInfo )
		{
			_ulFrequency = _psVorbisInfo->rate;
			_ulChannels = _psVorbisInfo->channels;
			if( _psVorbisInfo->channels == 1 )
			{
				_ulFormat = AL_FORMAT_MONO16;
				// Set BufferSize to 250ms (Frequency * 2 (16bit) divided by 4 (quarter of a second))
				_ulBufferSize = _ulFrequency >> 1;
				// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
				_ulBufferSize -= (_ulBufferSize % 2);
			}
			else if( _psVorbisInfo->channels == 2 )
			{
				_ulFormat = AL_FORMAT_STEREO16;
				// Set BufferSize to 250ms (Frequency * 4 (16bit stereo) divided by 4 (quarter of a second))
				_ulBufferSize = _ulFrequency;
				// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
				_ulBufferSize -= (_ulBufferSize % 4);
			}
			else if( _psVorbisInfo->channels == 4 )
			{
				_ulFormat = alGetEnumValue("AL_FORMAT_QUAD16");
				// Set BufferSize to 250ms (Frequency * 8 (16bit 4-channel) divided by 4 (quarter of a second))
				_ulBufferSize = _ulFrequency * 2;
				// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
				_ulBufferSize -= (_ulBufferSize % 8);
			}
			else if( _psVorbisInfo->channels == 6 )
			{
				_ulFormat = alGetEnumValue("AL_FORMAT_51CHN16");
				// Set BufferSize to 250ms (Frequency * 12 (16bit 6-channel) divided by 4 (quarter of a second))
				_ulBufferSize = _ulFrequency * 3;
				// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
				_ulBufferSize -= (_ulBufferSize % 12);
			}
		}
		else{
			qDebug() << "[Stream::startStreaming()]"<< " --------- ERROR: psVorbisInfo failed";
			_lastError = CS_ERROR_FILE_OGG;
			return false;
		}

		error = alGetError();
		if (_ulFormat == 0){
			qDebug() << "[Stream::startStreaming()]"<< " --------- ERROR: formal unknow";
			_lastError = CS_ERROR_FILE_OGG;
			return false;
		}
		
		if (_pDecodeBuffer){
			free(_pDecodeBuffer);
			_pDecodeBuffer = NULL;
		}

		_pDecodeBuffer = (char*)malloc(_ulBufferSize);
		if (!_pDecodeBuffer)
		{
			qDebug() << "[Stream::startStreaming()]"<< " --------- ERROR: DecodeBuffer failed to alocate memory";
			fn_ov_clear( _sOggVorbisFile );
			qDebug() << "[Stream::startStreaming()]"<< " -------------------- clear ogg file";
			_lastError = CS_ERROR_FILE_OGG;
			return false;
		}
		// Fill all the Buffers with decoded audio data from the OggVorbis file
		for (ALint i = 0; i < NUMBUFFERSOGG; i++)
		{
			_ulBytesWritten = DecodeOggVorbis( _sOggVorbisFile, _pDecodeBuffer, _ulBufferSize, _ulChannels );
			error = alGetError();
			if (_ulBytesWritten)
			{
				alBufferData( _uiBuffers[i], _ulFormat, _pDecodeBuffer, _ulBytesWritten, _ulFrequency );
				alSourceQueueBuffers( _uiSource, 1, &_uiBuffers[i] );
			}
			error = alGetError();
		}
		//CnotiLogManager::getSingleton().getLog(soundLog)->logMessage("first 4 buffer is decoded");
		_iTotalBuffersProcessed = 0;
		error = alGetError();
		//fclose(pOggVorbisFile);
		return true;
	}

/*!
	NOT IMPLEMENTED
*/
	bool Stream::save(const QString filename)
	{
		_lastError = CS_NOT_IMPLEMENT;
		return false;
	}

/*!
	Returns true if stream is empty, otherwise false.

	NOT IMPLEMENTED
*/
	bool Stream::isEmpty()
	{
		_lastError = CS_NOT_IMPLEMENT;
		return true;
	}

/*!
	NOT IMPLEMENTED
*/
	bool Stream::compareSound(SoundBase* second)
	{
		_lastError = CS_NOT_IMPLEMENT;
		return false;
	}

/*!
	NOT IMPLEMENTED
*/
	float Stream::percentPlay()
	{
		_lastError = CS_NOT_IMPLEMENT;
		return -1.0;
	}
	
/*!
	Updates the buffer and check if stream as ended.
*/
	void Stream::update()
	{
		int error = alGetError();
		//
		// Gets the buffer processed
		//
		_iBuffersProcessed = 0;
		alGetSourcei(_uiSource, AL_BUFFERS_PROCESSED, &_iBuffersProcessed);

		_iTotalBuffersProcessed += _iBuffersProcessed;
		//
		// For each processed buffer, remove it from the Source Queue, read next chunk of audio
		// data from disk, fill buffer with new data, and add it to the Source Queue
		//
		while( _iBuffersProcessed )
		{			
			//
			// Remove the Buffer from the Queue.  (uiBuffer contains the Buffer ID for the unqueued Buffer)
			//
			_uiBuffer = 0;
			alSourceUnqueueBuffers(_uiSource, 1, &_uiBuffer);
			//
			// Read more audio data (if there is any)
			//					
			_ulBytesWritten = DecodeOggVorbis( _sOggVorbisFile, _pDecodeBuffer, _ulBufferSize, _ulChannels );
			if (_ulBytesWritten)
			{
				//
				// Copy audio data to Buffer
				//
				alBufferData( _uiBuffer, _ulFormat, _pDecodeBuffer, _ulBytesWritten, _ulFrequency );
				//
				// Queue Buffer on the Source
				//
				alSourceQueueBuffers( _uiSource, 1, &_uiBuffer );
			}		
			_iBuffersProcessed--;
		} // end while iBuffersProcessed
		
		ALenum state; 
		alGetSourcei( _uiSource, AL_SOURCE_STATE, &state) ;
		//
		// if the state is stoped
		//
		if( isStopped() )
		{
            bool goingToStop = false;
			if( _loop && !_flagThreadSoundStopped )
			{				
				//
				// and the loop is true then start to play again
				//
				_streamingStarted = false;
				error = alGetError();
                if( startStreaming() ) // Nuno: Some ogg file are stucked in her
				{     
                    //
					// PLAY
					//
				    alSourcePlay(_uiSource );
				    error = alGetError();
                }
                else
				{
					error = alGetError();
                    goingToStop = true;
                }
			}
			else{	
                goingToStop = true;
            }
            if( goingToStop )
			{
				//
				// else stop the sound and return false
				//
				qDebug() << "[Stream::update()]"<< " -------------------- Stop sound" + _name;
                stopSound();
			}
		}
	}

/*!
	Decodes ogg vorbis files
*/
	unsigned long Stream::DecodeOggVorbis(OggVorbis_File *psOggVorbisFile, char *pDecodeBuffer, unsigned long ulBufferSize, unsigned long ulChannels)
	{
		int current_section;
		long lDecodeSize;
		unsigned long ulSamples;
		short *pSamples;

		unsigned long ulBytesDone = 0;
		while (1)
		{
			lDecodeSize = fn_ov_read(psOggVorbisFile, pDecodeBuffer + ulBytesDone, ulBufferSize - ulBytesDone, 0, 2, 1, &current_section);
			if (lDecodeSize > 0)
			{
				ulBytesDone += lDecodeSize;

				if (ulBytesDone >= ulBufferSize)
					break;
			}
			else
			{
				break;
			}
		}

		// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
		// however 6-Channels files need to be re-ordered
		if (ulChannels == 6)
		{		
			pSamples = (short*)pDecodeBuffer;
			for (ulSamples = 0; ulSamples < (ulBufferSize>>1); ulSamples+=6)
			{
				// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
				// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
				Swap(pSamples[ulSamples+1], pSamples[ulSamples+2]);
				Swap(pSamples[ulSamples+3], pSamples[ulSamples+5]);
				Swap(pSamples[ulSamples+4], pSamples[ulSamples+5]);
			}
		}

		return ulBytesDone;
	}
}

/*
	Reads an array of count elements, each one with a size of \a size bytes, from the \a datasource and stores them in the block of memory specified by \a ptr.
*/
size_t ov_read_func(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	return fread(ptr, size, nmemb, (FILE*)datasource);
}

/*
	Sets the position indicator associated with the \a datasource to a new position defined by adding \a offset to a reference position specified by \a whence.
*/
int ov_seek_func(void *datasource, ogg_int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

/*
	Closes the file associated with the \a datasource and disassociates it.
*/
int ov_close_func(void *datasource)
{
   return fclose((FILE*)datasource);
}

/*
	Returns the current value of the position indicator of the \a datasource.
*/
long ov_tell_func(void *datasource)
{
	return ftell((FILE*)datasource);
}

/*
	Swaps \a s1 with \a s2.
*/
void Swap(short &s1, short &s2)
{
	short sTemp = s1;
	s1 = s2;
	s2 = sTemp;
}
