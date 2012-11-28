#include <QString>
#include <QFile>

#include <QDebug>

#include "SoundBase.h"
#include "SoundManager.h"
#include "Melody.h"
#ifndef _WIN32
#include "sndfile.h"
#endif

namespace CnotiAudio
{
/*!
	Constructs an empty soundBase with the name \a name.
*/
	SoundBase::SoundBase(const QString name) :
		_iFrequency(22050)
	{
//		_type			= CS_SAMPLE;
		_lastError		= CS_NO_ERROR;
		_name			= name;

		_timer					= new QTime();
		_currTime				= 0;
		_pauseTime				= 0;
		_stopped				= true;
		_flagThreadSoundStopped	= false;
		_intensity				= 1.0;

		_sourcePos[0]	 = 0.0;
		_sourcePos[1]	= 0.0;
		_sourcePos[2]	= 0.0;

		_data		= 0;
		_size		= 0;

		_uiSource   = 0;

		_soundMgr	= SoundManager::instance();

		_logFile    = CnotiAudio::SoundManager::instance()->getLogFile();
	}

/*!
	Constructs a copy of other.
*/
	SoundBase::SoundBase(const SoundBase &other)
	{
//		this->_type				= other._type;
		this->_name				= other._name;
		this->_lastError		= other._lastError;

		this->_timer					= new QTime();
		this->_totalDuration			= other._totalDuration;
		this->_currTime					= other._currTime;
		this->_pauseTime				= other._pauseTime;
		this->_stopped					= other._stopped;
		this->_flagThreadSoundStopped	= other._flagThreadSoundStopped;
		this->_intensity				= other._intensity;
		this->_logFile					= other._logFile;

		_sourcePos[0]			= other._sourcePos[0];
		_sourcePos[1]			= other._sourcePos[1];
		_sourcePos[2]			= other._sourcePos[2];

		_iFrequency				= other._iFrequency;
		_data = new short[_size];
		memcpy(_data, (short*)(other._data), _size);
	}

/*!
	Constructs a copy of other, renaming to \a newName.
*/
	SoundBase::SoundBase(SoundBase &other, const QString newName)
	{
//		this->_type				= other._type;
		this->_lastError		= other._lastError;

		this->_timer			= new QTime();
		this->_currTime			= other._currTime;
		this->_totalDuration	= other._totalDuration;
		this->_pauseTime		= other._pauseTime;
		this->_stopped			= other._stopped;
		this->_flagThreadSoundStopped			= other._flagThreadSoundStopped;
		this->_intensity		= other._intensity;
		this->_logFile          = other._logFile;

		this->_name				= newName;

		_sourcePos[0]			= other._sourcePos[0];
		_sourcePos[1]			= other._sourcePos[1];
		_sourcePos[2]			= other._sourcePos[2];

		_iFrequency				= other._iFrequency;
		_data = new short[_size];
		memcpy(_data, (short*)(other._data), _size);
	}

/*!
	Destroyes the sound.
*/
	SoundBase::~SoundBase()
	{
		delete(_timer);
		terminate();
	}

/*!
   Saves the sound into a wav file named \a filename.

   Returns true if file was saved, otherwise false.
*/
	bool SoundBase::saveWav(const QString filename)
	{


#if defined(_WIN32) || defined(Q_WS_WIN) || defined(Q_WS_WIN32)

		//
		// update of data and size
		//
		short* dataSound = getData();
		if(dataSound == 0)
		{
			return false;
		}
		unsigned long sizeSound = getSize();
		if(sizeSound == 0)
		{
			return false;
		}

		typedef struct
		{
			char			szRIFF[4];
			long			lRIFFSize;
			char			szWave[4];
			char			szFmt[4];
			long			lFmtSize;
			WAVEFORMATEX		wfex;
			char			szData[4];
			long			lDataSize;
		} WAVEHEADER;
		WAVEHEADER		_waveheader;

		QString filenameWav = filename;
		if(!filenameWav.contains(".wav"))
		{
			filenameWav.append(".wav");
		}

		qDebug()<< "[SoundBase::saveWav]" << filenameWav;

		QFile fp( filenameWav );

		if( !fp.open( QIODevice::WriteOnly ) ) // Open file
		{
			return false;
		}

		//
		// Set the file header info
		//
		sprintf( _waveheader.szRIFF, "RIFF" );
		_waveheader.lRIFFSize = sizeSound + sizeof(WAVEHEADER) - 8;
		sprintf( _waveheader.szWave, "WAVE" );
		sprintf( _waveheader.szFmt, "fmt " );
		_waveheader.lFmtSize = sizeof(WAVEFORMATEX);
		_waveheader.wfex.nChannels = CS_NUMBERCHANNEL;
		_waveheader.wfex.wBitsPerSample = 16;
		_waveheader.wfex.wFormatTag = WAVE_FORMAT_PCM;
		_waveheader.wfex.nSamplesPerSec = _iFrequency;
		_waveheader.wfex.nBlockAlign = _waveheader.wfex.nChannels * (_waveheader.wfex.wBitsPerSample / 8);
		_waveheader.wfex.nAvgBytesPerSec = _waveheader.wfex.nSamplesPerSec * _waveheader.wfex.nBlockAlign;
		_waveheader.wfex.cbSize = 0;
		sprintf( _waveheader.szData, "data" );
		_waveheader.lDataSize = sizeSound + sizeof(WAVEHEADER) - 8;

		//
		// Write header into file
		//
		fp.write( (char*)&_waveheader, sizeof( WAVEHEADER ) );

		//
		// Write data into file
		//
		fp.write( (char*)dataSound, sizeSound );
		fp.close();

#else
		//
		// Set the file header info
		//
		SNDFILE	*file;
		SF_INFO	sfInfo;
		memset (&sfInfo, 0, sizeof (sfInfo)) ;
		sfInfo.samplerate = _iFrequency;
		sfInfo.channels   = CS_NUMBERCHANNEL;
		sfInfo.format     = (SF_FORMAT_WAV | SF_FORMAT_PCM_16);

		//unsigned long sizeSound = getSize();
		unsigned long sizeSound = getSize() / SF_FORMAT_PCM_16;    // Returns 2x the size but in previous code it was correct
		sfInfo.frames = sizeSound;

		//
		// Check if a set of parameters in the SF_INFO struct is valid
		//
		if( !sf_format_check( &sfInfo ) )
		{
			qDebug() << "[SoundBase::saveWav]" << " Error SF_INFO parameters";
			return false;
		}

		//
		// Open the file
		//
		QString filenameWav = filename + ".wav";
		if( !( file = sf_open( filenameWav.toStdString().c_str(), SFM_WRITE, &sfInfo ) ) )
		{
			int sfError = sf_error( file ) ;
			qDebug() << "[SoundBase::saveWav]" << " Error opening file";
			return false;
		}

		//
		// get the data to save in to the file
		//
		short* dataSound = getData();

		//
		// Write the data to the file
		//
		if( sf_write_short( file, dataSound, sizeSound) != sizeSound )
		{
			//puts( sf_strerror(file) ) ;
			qDebug() << "[SoundBase::saveWav]" << " Error writing file";
			return false;
		}

		sf_close(file) ;
#endif
		return true;
	}

/*!
   Saves the sound in to a mp3 file named \a filename, with a rate value od \a minimumRate.

   First saves the sound into a wav file. Then using the wav file
   creates the mp3 data and saves that data into the mp3 file.

   To keep the wav file \a deleteWav must be false.

   Returns true if file was saved, otherwise false.
*/
	bool SoundBase::saveMp3(const QString filename, int minimumRate, bool deleteWav)
	{
		qDebug()<< "[SoundBase::saveMp3]";
		QString newFileName = filename;
		newFileName.remove(".mp3");
		//
		// Save to a wav file
		//
		QFile f;
		f.setFileName( newFileName + ".wav" );
		if( !f.exists() )
		{
			if( !saveWav( newFileName ) )
			{
				return false;
			}
		}

		//
		// Code to MP3
		//
		bool result = _soundMgr->lameMp3( newFileName, minimumRate, _iFrequency );

		//
		// Delete wav file
		//
		if( deleteWav )
		{
			f.remove();
		}

		return result;
	}

/*!
   Changes the intensity value that the sound will be played.

   The new volume will be \a intensity.

   The intensity value varies from 0 to 1. If \a intensity is smaller or bigger then the limit, the intensity is adjust to this values.
*/
	void SoundBase::setVolume( float intensity )
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
		// If is playing changes the intensity on the fly
		//
		if( isPlaying() )
		{
			alGetError();
			alSourcef(_uiSource, AL_GAIN, _intensity);
			if( alGetError() != AL_NO_ERROR )
			{
				qDebug() << "[SoundBase::setIntensity]"<< "------------  Error: updating volume -> " << _name;
			}
		}
	}

/*!
   Changes the position \a x, \a y and \a z, where the sound will be played.
*/
	void SoundBase::setSourcePosition(float x, float y, float z)
	{
		_sourcePos[0] = x;
		_sourcePos[1] = y;
		_sourcePos[2] = z;
	}

/*!
   Sound cicle.
*/
	void SoundBase::run()
	{
		forever
		{
			// Do waht it needs to do
			update();
			// Waits
			msleep(CS_REFRESH);
			// Check if is to stop the thread
			_mutex.lock();
			if( _flagThreadSoundStopped )
			{
				_flagThreadSoundStopped = false;
				_mutex.unlock();
				qDebug() << "[SoundBase::run()]"<< "------------  Stopping thread for file: " << _name;
				break;
			}
			_mutex.unlock();
		}
	}

/*!
	Returns true if sound is playing, otherwise false.
*/
	bool SoundBase::isPlaying()
	{
		ALenum state;
		alGetSourcei(_uiSource, AL_SOURCE_STATE, &state);
		if(state == AL_PLAYING)
			return true;
		else
			return false;
	}

/*!
	Returns true if sound is stopped, otherwise false.
*/
	bool SoundBase::isStopped()
	{
		return !isPlaying() && !isPaused();
	}

/*!
	Returns true if sound is paused, otherwise false.
*/
	bool SoundBase::isPaused()
	{
		ALenum state;
		alGetSourcei(_uiSource, AL_SOURCE_STATE, &state);
		if(state == AL_PAUSED)
			return true;
		else
			return false;
	}

//=====================================================//
//                    GETTERS                          //
//=====================================================//
/*!
	Returns the name od the sound.
*/
	QString SoundBase::getName()
	{
		return _name;
	}

/*!
	Returns the code of the last error occurred .
*/
	CnotiErrorSound SoundBase::getLastError()
	{
		return _lastError;
	}

/*!
	Returns the source x position.
*/
	float SoundBase::getSourceX()
	{
		return _sourcePos[0];
	}

/*!
	Returns the source y position.
*/
	float SoundBase::getSourceY()
	{
		return _sourcePos[1];
	}

/*!
	Returns the source z position.
*/
	float SoundBase::getSourceZ()
	{
		return _sourcePos[2];
	}

/*!
	Returns the sound intensity
*/
	float SoundBase::getIntensity()
	{
		return _intensity;
	}

/*!
	Returns the sound data.
*/
	short* SoundBase::getData()
	{
		return _data;
	}

/*!
	Returns the sound size.
*/
	unsigned long SoundBase::getSize()
	{
		return _size;
	}

/*!
	Returns the sound frequency.
*/
	ALint SoundBase::getFrequency()
	{
		return _iFrequency;
	}

/*!
	Returns the sound frequency.
*/
	void SoundBase::setFrequency(ALint frequency)
	{
		_iFrequency = frequency;
	}

/*!
	Initializes the connections of the sounds with the soundManager
*/
	void SoundBase::connectToSoundManager()
	{
		connect( this, SIGNAL(soundStopped(QString)), _soundMgr, SIGNAL(soundStopped(QString)), Qt::DirectConnection);
		connect( this, SIGNAL(soundPlaying(QString)), _soundMgr, SIGNAL(soundPlaying(QString)), Qt::DirectConnection);
		connect( this, SIGNAL(soundPaused(QString)), _soundMgr, SIGNAL(soundPaused(QString)), Qt::DirectConnection);
	}

	float SoundBase::getDuration()
	{
		return getSize() * 1.0 / (_iFrequency * ( 16 / 8));
	}
}
