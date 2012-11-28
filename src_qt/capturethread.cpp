#include "capturethread.h"
#include "CnotiAudio.h"
#include "SoundManager.h"
#include <QString>
#include <QStringList>

#include "LogManager.h"

#include "DaisyFilter/DaisyFilter.h"

#include <QDebug>

//#include "FIR.h"
//#include "Signals.h"

//TODO: Save into a buffer

namespace CnotiAudio
{

	CaptureThread::CaptureThread(QObject *parent)
	{
		_capturing            = false;
		_noiseReductionActive = false;

		initializeSound();
		
	//	sample = -1;	

		_logFile = SoundManager::instance()->getLogFile();
	}

	CaptureThread::~CaptureThread()
	{
		stopCapture();
	}


	void CaptureThread::initializeSound()
	{
		
		_pContext = NULL;
		_pDevice = NULL;	

		// Check for Capture Extension support
		_pContext = alcGetCurrentContext();
		_pDevice = alcGetContextsDevice( _pContext );
		if( alcIsExtensionPresent( _pDevice, "ALC_EXT_CAPTURE") == AL_FALSE ) {
			return;
		}

		// Get list of available Capture Devices
		const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
		if ( pDeviceList )
		{
			while (*pDeviceList)
			{
				pDeviceList += strlen(pDeviceList) + 1;
			}
		}

		// Get the name of the 'default' capture device
		_szDefaultCaptureDevice = alcGetString( NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER );

		//pCaptureDevice = alcCaptureOpenDevice( szDefaultCaptureDevice, 44100, AL_FORMAT_MONO16, BUFFERSIZE );
		this->changeDevice( QString( _szDefaultCaptureDevice ) );
	}


	void CaptureThread::run()
	{
		_captureMutex.lock();
		
		int volume, n; // variables for calculating the mean of the volume
#if defined( __WIN32__ ) || defined( _WIN32 )
		int sampleSize = BUFFERSIZE / _sWaveHeader.wfex.nBlockAlign;
#else
		int sampleSize = BUFFERSIZE / _nBlockAlign;
#endif
		while( _capturing ) 
		{
			// Release some CPU time ...
#if defined( __WIN32__ ) || defined( _WIN32 )
			Sleep( 1 );
#else
			usleep( 10 );
#endif
			// Find out how many samples have been captured
			alcGetIntegerv( _pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &_iSamplesAvailable );

			// When we have enough data to fill our BUFFERSIZE byte buffer, grab the samples
			if( _iSamplesAvailable > ( sampleSize ) ) 
			{
				//
				// Consume Samples
				//
				alcCaptureSamples( _pCaptureDevice, _buffer, sampleSize );
				//
				// Write the audio data to a file
				//
				writeAudioBufferToFile( BUFFERSIZE );
				//
				// Record total amount of data recorded
				//
				_iDataSize += BUFFERSIZE;
			}

			// Calculates the mean of the volume, from the samples on the buffer
			volume = 0;
			n = ( ( BUFFERSIZE < _iSamplesAvailable ) ? _iSamplesAvailable : BUFFERSIZE );
			if( n > 0 ) 
			{
				for( int i = 0; i < n; i++ ) 
				{
					volume += _buffer[i];
				}
				volume /= n;
			}
				
			emit captureVolume( volume );
		}
		_captureMutex.unlock();
	}

	void CaptureThread::startCapture( const QString &filename )
	{
		if( _pCaptureDevice )	
		{
			if( _capturing ) 
			{
				return;
			}

#if defined( __WIN32__ ) || defined( _WIN32 )
			// Create / open a file for the captured data
			_pFile = fopen(filename.toLatin1(), "wb");

			// Prepare a WAVE file header for the captured data
			sprintf(_sWaveHeader.szRIFF, "RIFF");
			_sWaveHeader.lRIFFSize = 0;
			sprintf(_sWaveHeader.szWave, "WAVE");
			sprintf(_sWaveHeader.szFmt, "fmt ");
			_sWaveHeader.lFmtSize = sizeof(WAVEFORMATEX);		
			_sWaveHeader.wfex.nChannels = 1;
			_sWaveHeader.wfex.wBitsPerSample = 16;
			_sWaveHeader.wfex.wFormatTag = WAVE_FORMAT_PCM;
			_sWaveHeader.wfex.nSamplesPerSec = 44100;
			_sWaveHeader.wfex.nBlockAlign = _sWaveHeader.wfex.nChannels * _sWaveHeader.wfex.wBitsPerSample / 8;
			_sWaveHeader.wfex.nAvgBytesPerSec = _sWaveHeader.wfex.nSamplesPerSec * _sWaveHeader.wfex.nBlockAlign;
			_sWaveHeader.wfex.cbSize = 0;
			sprintf(_sWaveHeader.szData, "data");
			_sWaveHeader.lDataSize = 0;

			fwrite(&_sWaveHeader, sizeof(WAVEHEADER), 1, _pFile);
#else
			// Prepare a WAVE file header for the captured data
//			memset (&sfInfo, 0, sizeof (sfInfo)) ;
//			sfInfo.samplerate = 44100;
//			sfInfo.channels   = CS_NUMBERCHANNEL;
//			sfInfo.format     = (SF_FORMAT_WAV | SF_FORMAT_PCM_16);

			//unsigned long sizeSound = getSize();
			//unsigned long sizeSound = getSize() / SF_FORMAT_PCM_16;    // Returns 2x the size but in previous code it was correct
			//sfInfo.frames = sizeSound;
			
			// Check if a set of parameters in the SF_INFO struct is valid
//			if( !sf_format_check( &sfInfo ) )
//			{
				//CnotiLogManager::getSingleton().getLog(_logFile)->logMessage("SoundBase::saveWav ----------- Error SF_INFO parameters");
//				return;
//			}

			// Create / open a file for the captured data
			//std::string filenameWav = filename + ".wav";
//			if( !( pFile = sf_open( filename.toStdString().c_str(), SFM_WRITE, &sfInfo ) ) )
//			{
//				int sfError = sf_error( pFile ) ;
				//CnotiLogManager::getSingleton().getLog(_logFile)->logMessage("SoundBase::saveWav ----------- Error opening file");
//				return;
//			}

//			_nBlockAlign = CS_NUMBERCHANNEL * SF_FORMAT_PCM_16;
#endif
			// Start audio capture
			alcCaptureStart( _pCaptureDevice );
			_capturing = true;
			_iDataSize = 0;
			start();
		}
		else 
		{
			emit signalCaptureEnded();
		}
	}

	void CaptureThread::stopCapture()
	{
		qDebug() << "[CaptureThread::stopCapture()]";
		if( _pCaptureDevice && _capturing )
		{
			//
			// Stop capture
			//
			_capturing = false;
			_captureMutex.lock();
			alcCaptureStop( _pCaptureDevice );

			//
			// Check if any Samples haven't been consumed yet
			//
			alcGetIntegerv( _pCaptureDevice, ALC_CAPTURE_SAMPLES, 1, &_iSamplesAvailable) ;
#if defined( __WIN32__ ) || defined( _WIN32 )
			int sampleSize = BUFFERSIZE / _sWaveHeader.wfex.nBlockAlign;
#else
                        int sampleSize = BUFFERSIZE / _nBlockAlign;
#endif
			while( _iSamplesAvailable )
			{
#if defined( __WIN32__ ) || defined( _WIN32 )
				if( _iSamplesAvailable > ( sampleSize ) )
				{
					alcCaptureSamples( _pCaptureDevice, _buffer, sampleSize );
					//
					//	Write to file
					//
					writeAudioBufferToFile( BUFFERSIZE );
					//fwrite( outputBuffer, BUFFERSIZE, 1, _pFile );
					//fwrite( _buffer, BUFFERSIZE, 1, _pFile );
					_iSamplesAvailable -= sampleSize;
					_iDataSize += BUFFERSIZE;
				}
				else
				{
					qDebug() << "[CaptureThread::stopCapture()]"<< " ----------- last sample size: " << _iSamplesAvailable;
					alcCaptureSamples(_pCaptureDevice, _buffer, _iSamplesAvailable);
					//
					//	Write to file
					//
					int writeSize = _iSamplesAvailable * _sWaveHeader.wfex.nBlockAlign;
					writeAudioBufferToFile( writeSize );
					//fwrite(_buffer, _iSamplesAvailable * _sWaveHeader.wfex.nBlockAlign, 1, _pFile);
					_iDataSize += writeSize;
					_iSamplesAvailable = 0;
				}
#else
				if( _iSamplesAvailable > sampleSize )
				{
					alcCaptureSamples( _pCaptureDevice, _buffer, sampleSize );					
					sf_write_short( _pFile, (short *)_buffer, BUFFERSIZE );
					_iSamplesAvailable -= sampleSize;
					_iDataSize += BUFFERSIZE;
				}
				else
				{
					sf_write_short( _pFile, (short *)_buffer, BUFFERSIZE);
					_iDataSize += _iSamplesAvailable * _nBlockAlign;
					_iSamplesAvailable = 0;
				}
#endif
			}

			_captureMutex.unlock();

#if defined( __WIN32__ ) || defined( _WIN32 )
			// Fill in Size information in Wave Header
			fseek( _pFile, 4, SEEK_SET );
			_iSize = _iDataSize + sizeof(WAVEHEADER) - 8;
			fwrite(&_iSize, 4, 1, _pFile);
			fseek(_pFile, 42, SEEK_SET);
			fwrite(&_iDataSize, 4, 1, _pFile);

			fclose( _pFile );
#else
			sf_close( _pFile ) ;
			terminate();
#endif
			emit signalCaptureEnded();
			emit captureVolume( 0 );
		}
	}

	QStringList CaptureThread::deviceList()
	{
		QStringList list;

		const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
		
		if (pDeviceList)
		{
			while (*pDeviceList)
			{
				list << QString::fromLatin1(pDeviceList);
				pDeviceList += strlen(pDeviceList) + 1;
			}
		}

		return list;
	}

	const QString CaptureThread::getDevice()
	{
		return _cDeviceName;

		//QString deviceName = "";
		//if( pCaptureDevice )
		//{
		//	const ALchar* devName = alcGetString( pCaptureDevice, ALC_CAPTURE_DEVICE_SPECIFIER );	// Gives error in debug mode
		//	deviceName = QString::fromLatin1( devName );
		//}

		//return deviceName;
	}

	void CaptureThread::changeDevice( const QString &deviceName )
	{
		if( !this->_capturing ) {
			this->_pCaptureDevice = alcCaptureOpenDevice( deviceName.toLatin1(), 44100, AL_FORMAT_MONO16, BUFFERSIZE );
			_cDeviceName = deviceName;
		}
	}

	bool CaptureThread::isCapturing()
	{
		return this->_capturing;
	}

	void CaptureThread::applyNoiseReduction( float* outBuffer, int bufferSize )
	{
		// Deleted for using in LittleMozart
	}

	void CaptureThread::writeAudioBufferToFile( int bufferWriteSize )
	{
		//
		//	Apply noise reduction filter
		//
//		float outputBuffer[ BUFFERSIZE ];
//		//applyNoiseReduction( outputBuffer, bufferWriteSize );


//		DaisyFilter *iirFilter = DaisyFilter::SinglePoleIIRFilter(0.3f);
//		float input = 0;
////_noiseReductionActive = true;

//		//float outBuffer[BUFFERSIZE];
//		//ALchar outBuffer[BUFFERSIZE];
//		QByteArray* outBuffer= new QByteArray( _buffer, BUFFERSIZE );
		
//		//for( int i = 0; i < bufferWriteSize ; i++ )
//		//{
//		//	//outBuffer[i] = (float)_buffer[i];
//		//	float aux = (short)_buffer[i]; // Cast to float directly doesn't convert correctly
//		//	if( _noiseReductionActive )
//		//	{
//		//		//outBuffer[i] = iirFilter->Calculate( aux );
//		//	}
//		//	else
//		//	{
//		//		outBuffer[i] = aux;
//		//	}
//		//}
#if defined( __WIN32__ ) || defined( _WIN32 )
		fwrite( _buffer, bufferWriteSize, 1, _pFile );
//		fwrite( *outBuffer, bufferWriteSize, 1, _pFile );
//		_bufferList.append( outBuffer );
#else
                sf_write_short( _pFile, (short *)_buffer, BUFFERSIZE );
#endif
		emit signalSampleCaptured();
	}

	QByteArray* CaptureThread::getBufferListData( int index )
	{
		if( index >= _bufferList.size() || index < 0 )
		{
			return 0;
		}
		return _bufferList.at( index );
	}
}
