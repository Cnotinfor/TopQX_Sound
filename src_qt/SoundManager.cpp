#include <QFile>
#include <QSettings>
#include <string>

#include "SoundManager.h"
#include "SoundBase.h"
#include "Sample.h"
#include "Stream.h"
#include "Sound.h"
#include "Music.h"
#include "Note.h"

#include "capturethread.h"
#include "SourcePool.h"
#include "notemisc.h"

//#include <windows.h>
#include <stdio.h>
//#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <math.h>

#include <QString>
#include <QDir>
#include <QDebug>

#ifdef _WIN32
#include "BladeMP3EncDLL.h"
/**
* lame mp3 constants
*/

BEINITSTREAM		beInitStream=NULL;
BEENCODECHUNK		beEncodeChunk=NULL;
BEDEINITSTREAM		beDeinitStream=NULL;
BECLOSESTREAM		beCloseStream=NULL;
BEVERSION			beVersion=NULL;
BEWRITEVBRHEADER	beWriteVBRHeader=NULL;
BEWRITEINFOTAG		beWriteInfoTag=NULL;
#else
#include "LAME/lame.h"
#include "sndfile.h"
#endif

//CnotiAudio::SoundManager CnotiAudio::SoundManager::ms_Singleton;

//template<> CnotiAudio::SoundManager* Singleton<CnotiAudio::SoundManager>::_Singleton = 0;	// null instance pointer

namespace CnotiAudio
{
/*!
	Constructs a soundManager.

	Before using the Sound Manager is necessary initialize it.

	\sa init()
*/
	SoundManager::SoundManager():
		_sourcePool(NULL)
	{
		_lastError	= CS_NO_ERROR;
		_pDevice = NULL;
		_hopBuffer = NULL;
		_bigBuffer = NULL;
		isInitAl = false;
		isInitOgg = false;
		isReleased = false;
		_captureThread = NULL;
		_noteMisc = NULL;
	}

/*!
	Destroyes the soundManager.
*/
	SoundManager::~SoundManager()
	{
		qDebug() << "[SoundManager::~SoundManager()]";
		if( !isReleased )
		{
			release();
		}
		qDebug() << "[SoundManager delete]";
	}

/*!
	Inits the soundManager.

	If \a sourcePoolSize is given, then the pool size will be created to contain a max of
	\a sourcePoolSize sources, By default is created a 16 sources pool.
*/
	void SoundManager::init (QString appName)
	{
		_appName = appName;
		//
		//	Create pointer to class NoteMisc.
		//
		_noteMisc = new NoteMisc();
		//
		// Ceate Log
		//
		QString s;
#ifdef _WIN32
		QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
		settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
		s = settings.value("AppData").toString();
		QDir pathResources( s + "\\Imagina\\" );
#else
		s = QDir::homePath();
		QDir pathResources( s + "/.Imagina/" );
#endif
		QString logFile = getLogFile();
		qDebug() << "[SoundManager::init]";
	}

/*!
	Releases objects used.
*/
	bool SoundManager::release()
	{
		qDebug() << "[SoundManager::release]";
		if( isReleased )
		{
			qDebug() << "[SoundManager::release] Already released.";
			return isReleased;
		}

		if( isInitAl )
		{
#ifdef _WIN32
			ALFWShutdown();
//#else
//				qDebug() << "[SoundManager::released] ---------  AL SHUTDOWN IS NOT NECESSARY?!?!?");
#endif
		}

		if( _bigBuffer != NULL )
		{
			delete( _bigBuffer );
		}

		if( _hopBuffer != NULL )
		{
			delete( _hopBuffer );
		}

		if( _captureThread != NULL )
		{
			delete( _captureThread );
		}

		if( _noteMisc != NULL )
		{
			delete( _noteMisc );
		}

		releaseAllSound();

		isReleased = true;
		return isReleased;
	}


/*!
	Encodes a wav file into mp3 and saves the mp3 data into \a filename with the extension ".mp3".
*/
	bool SoundManager::lameMp3(const QString& filename, int minimumRate, int frequency)
	{
		qDebug() << "[SoundManager::lameMp3()]";

#if defined(__WIN32__) || defined(_WIN32) || defined(Q_WS_WIN) || defined(Q_WS_WIN32)
                QFile		pFileOut		=NULL;
                QFile		pFileIn			=NULL;

		HINSTANCE	hDLL			=NULL;
		BE_VERSION	Version			={0,};
		BE_CONFIG	beConfig		={0,};

		DWORD		dwSamples		=0;
		DWORD		dwMP3Buffer		=0;
		HBE_STREAM	hbeStream		=0;
		BE_ERR		err				=0;

		PBYTE		pMP3Buffer		=NULL;
		PSHORT		pWAVBuffer		=NULL;

		//
		// Setup the file names
		//
		QString mp3Filename = filename + ".mp3";
		QString wavFilename = filename + ".wav";

		qDebug() << wavFilename;
		qDebug() << mp3Filename;

		//
		// Load lame_enc.dll library (Make sure though that you set the
		// project/settings/debug Working Directory correctly, otherwise the DLL can't be loaded
		//
		hDLL = LoadLibraryA("lame_enc.dll");

		if( NULL == hDLL )
		{
			fprintf(stderr,"Error loading lame_enc.DLL");
			qWarning() << "[SoundManager::lameMp3] - lame_enc.dll not found";
			return false;
		}
		//
		// Get Interface functions from the DLL
		//
		beInitStream	= (BEINITSTREAM) GetProcAddress(hDLL, TEXT_BEINITSTREAM);
		beEncodeChunk	= (BEENCODECHUNK) GetProcAddress(hDLL, TEXT_BEENCODECHUNK);
		beDeinitStream	= (BEDEINITSTREAM) GetProcAddress(hDLL, TEXT_BEDEINITSTREAM);
		beCloseStream	= (BECLOSESTREAM) GetProcAddress(hDLL, TEXT_BECLOSESTREAM);
		beVersion		= (BEVERSION) GetProcAddress(hDLL, TEXT_BEVERSION);
		beWriteVBRHeader= (BEWRITEVBRHEADER) GetProcAddress(hDLL,TEXT_BEWRITEVBRHEADER);
		beWriteInfoTag  = (BEWRITEINFOTAG) GetProcAddress(hDLL,TEXT_BEWRITEINFOTAG);

		//
		// Check if all interfaces are present
		//
		if(!beInitStream || !beEncodeChunk || !beDeinitStream || !beCloseStream || !beVersion || !beWriteVBRHeader)
		{
			qWarning() << "[SoundManager::lameMp3] - Unable to get LAME interfaces";
			return false;
		}
		//
		// Get the version number
		//
		beVersion( &Version );
		//
		// Try to open the WAV file, be sure to open it as a binary file!
		//
		pFileIn.setFileName( wavFilename );
		if(!pFileIn.open( QIODevice::ReadOnly ) )
		{
			qWarning() << "[SoundManager::lameMp3] - Error opening" << wavFilename;
			return false;
		}
		//
		// Open MP3 file
		//
		pFileOut.setFileName( mp3Filename );
		if(!pFileOut.open( QIODevice::WriteOnly ))
		{
			qWarning() << "[SoundManager::lameMp3] - Error creating" << mp3Filename;
			return false;
		}

		memset(&beConfig,0,sizeof(beConfig));					// clear all fields
		//
		// use the LAME config structure
		//
		beConfig.dwConfig = BE_CONFIG_LAME;
		//
		// this are the default settings for filename.wav
		//
		beConfig.format.LHV1.dwStructVersion	= 1;
		beConfig.format.LHV1.dwStructSize		= sizeof(beConfig);
		beConfig.format.LHV1.dwSampleRate		= frequency;			// INPUT FREQUENCY
		beConfig.format.LHV1.dwReSampleRate		= 0;					// DON"T RESAMPLE
		if(CS_NUMBERCHANNEL>1)
			beConfig.format.LHV1.nMode			= BE_MP3_MODE_STEREO;		// OUTPUT IN STREO
		else
			beConfig.format.LHV1.nMode			= BE_MP3_MODE_MONO;		// OUTPUT IN STREO
		beConfig.format.LHV1.dwBitrate			= minimumRate;			// MINIMUM BIT RATE
		beConfig.format.LHV1.nPreset			= LQP_R3MIX;			// QUALITY PRESET SETTING
		beConfig.format.LHV1.dwMpegVersion		= MPEG1;				// MPEG VERSION (I or II)
		beConfig.format.LHV1.dwPsyModel			= 0;					// USE DEFAULT PSYCHOACOUSTIC MODEL
		beConfig.format.LHV1.dwEmphasis			= 0;					// NO EMPHASIS TURNED ON
		beConfig.format.LHV1.bOriginal			= TRUE;					// SET ORIGINAL FLAG
		beConfig.format.LHV1.bWriteVBRHeader	= TRUE;					// Write INFO tag

	//	beConfig.format.LHV1.dwMaxBitrate		= 320;					// MAXIMUM BIT RATE
	//	beConfig.format.LHV1.bCRC				= TRUE;					// INSERT CRC
	//	beConfig.format.LHV1.bCopyright			= TRUE;					// SET COPYRIGHT FLAG
	//	beConfig.format.LHV1.bPrivate			= TRUE;					// SET PRIVATE FLAG
	//	beConfig.format.LHV1.bWriteVBRHeader	= TRUE;					// YES, WRITE THE XING VBR HEADER
	//	beConfig.format.LHV1.bEnableVBR			= TRUE;					// USE VBR
	//	beConfig.format.LHV1.nVBRQuality		= 5;					// SET VBR QUALITY
		beConfig.format.LHV1.bNoRes				= TRUE;					// No Bit resorvoir

	// Preset Test
	//	beConfig.format.LHV1.nPreset			= LQP_PHONE;

		//
		// Init the MP3 Stream
		//
		err = beInitStream(&beConfig, &dwSamples, &dwMP3Buffer, &hbeStream);
		if(err != BE_ERR_SUCCESSFUL)
		{
			qWarning() << "[SoundManager::lameMp3] - Error opening encoding stream";
			return false;
		}
		//
		// Allocate MP3 buffer
		//
		pMP3Buffer = new BYTE[dwMP3Buffer];
		//
		// Allocate WAV buffer
		//
		pWAVBuffer = new SHORT[dwSamples];
		//
		// Check if Buffer are allocated properly
		//
		if(!pMP3Buffer || !pWAVBuffer)
		{
			qWarning() << "[SoundManager::lameMp3] - Out of memory";
			return false;
		}

		DWORD dwRead=0;
		DWORD dwWrite=0;
		DWORD dwDone=0;
		DWORD dwFileSize=0;

		//
		// Get the file size
		//
		dwFileSize=pFileIn.size();
		//
		// Seek back to start of WAV file,
		// but skip the first 44 bytes, since that's the WAV header
		//
		pFileIn.seek( 44 );

		//
		// Convert All PCM samples
		//
		while ( (dwRead = pFileIn.read( (char*) pWAVBuffer, sizeof(SHORT) * dwSamples) ) > 0 )
		{
			//
			// Encode samples
			//
			err = beEncodeChunk(hbeStream, dwRead / sizeof(SHORT), pWAVBuffer, pMP3Buffer, &dwWrite);
			if(err != BE_ERR_SUCCESSFUL)
			{
				beCloseStream(hbeStream);
				qWarning() << "[SoundManager::lameMp3] - beEncodeChunk() failed";
				return false;
			}
			//
			// write dwWrite bytes that are returned in tehe pMP3Buffer to disk
			//
			if(pFileOut.write( (char*) pMP3Buffer, dwWrite ) != dwWrite)
			{
				qWarning() << "[SoundManager::lameMp3] - Output file write error";
				return false;
			}

			dwDone += dwRead*sizeof(SHORT);
		}
		//
		// Deinit the stream
		//
		err = beDeinitStream(hbeStream, pMP3Buffer, &dwWrite);
		if(err != BE_ERR_SUCCESSFUL)
		{
			qDebug() << "[beExitStream failed]";
			beCloseStream(hbeStream);
			//fprintf(stderr,"beExitStream failed (%lu)", err);
			return false;
		}
		//
		// Are there any bytes returned from the DeInit call?
		// If so, write them to disk
		//
		if( dwWrite )
		{
			if( pFileOut.write( (char*) pMP3Buffer, dwWrite ) != dwWrite )
			{
				qWarning() << "[SoundManager::lameMp3] - Output file write error";
				return false;
			}
		}
		//
		// close the MP3 Stream
		//
		beCloseStream( hbeStream );
		//
		// Delete WAV buffer
		//
		delete [] pWAVBuffer;
		//
		// Delete MP3 Buffer
		//
		delete [] pMP3Buffer;
		//
		// Close input file
		//
		pFileIn.close();
		//
		// Close output file
		//
		pFileOut.close();

//		if ( beWriteInfoTag )
//		{
//			// Write the INFO Tag
//			beWriteInfoTag( hbeStream, strFileOut );
//		}
//		else
//		{
//			beWriteVBRHeader( strFileOut );
//		}

		// Were done, return OK result
		return true;
#else
		FILE*		pFileOut		=NULL;

		char		strFileIn[255]	={'0',};
		char		strFileOut[255]	={'0',};
		//
		// Initialize the encoder.  sets default for all encoder parameters
		//
		lame_global_flags *gfp;
		gfp = lame_init();
		if( gfp == NULL )
		{
			qWarning() << "[SoundManager::lameMp3] - Error initialising lame";
			return false;
		}
		//
		// Setup the file names
		//
				strcpy(strFileIn ,filename.toStdString().c_str());
				strcpy(strFileOut,filename.toStdString().c_str());
		//
		// Add mp3/wav extention
		//
		strcat(strFileOut,".mp3");
		strcat(strFileIn,".wav");
		//
		// Open MP3 file
		//
		pFileOut= fopen(strFileOut,"wb+");
		if(pFileOut == NULL)
		{
			qWarning() << "[SoundManager::lameMp3] - Error creating" << QString(strFileOut);
			return false;
		}
		//
		// The default (if you set nothing) is a  J-Stereo, 44.1khz 128kbps CBR mp3 file at quality 5
		// Change some parameters
		//
		lame_set_num_channels(gfp,1);
		lame_set_in_samplerate(gfp, frequency);
		lame_set_brate(gfp, minimumRate);
//		lame_set_mode(gfp,3);
//		lame_set_quality(gfp,2);   /* 2=high  5 = medium  7=low */
		//
		// Set more internal configuration based on data provided above,
		//as well as checking for problems
		//
		if( lame_init_params(gfp) < 0 )
		{
			qWarning() << "[SoundManager::lameMp3] - Error initialising parameters";
			lame_close(gfp);
			return false;
		}

		SNDFILE	*sndFileIn;
		SF_INFO	sfInfo;
		if( !( sndFileIn = sf_open( strFileIn, SFM_READ, &sfInfo ) ) )
		{
			qWarning() << "[SoundManager::lameMp3] - Error opening file";
			return false;
		}

// TODO: Find better way to set buffer size or
//       another way to load /etc. so that the buffer can be smaller
		int	mDuration = sfInfo.frames / sfInfo.samplerate;
		int samples_to_read = mDuration * sfInfo.samplerate + 1000;
        int	buffer[5000000];
		int samples_read = sf_read_int( sndFileIn, buffer, samples_to_read );

		sf_close(sndFileIn) ;

//		for (int i = 0; i < samples_read; i++)
//			buffer[i] <<= (8 * sizeof(int) - 16); //16
		//
		// Encode some data
		//
        unsigned char mp3Buffer[5000000];
		int ret_bytes = lame_encode_buffer_int(gfp, buffer, buffer, samples_read, mp3Buffer, sizeof(mp3Buffer));

		if( ret_bytes < 0 )
		{
			qWarning() << "[SoundManager::lameMp3] - Error encoding";
			lame_close(gfp);
			return false;
		}
		//
		// Write to out file
		//
		int ret_write = fwrite( mp3Buffer, 1, ret_bytes, pFileOut );
		if( ret_write != ret_bytes )
		{
			qWarning() << "[SoundManager::lameMp3] - Error saving to " << QString(strFileOut);
			return false;
		}
		//
		// Flush the buffers, this may return a final few mp3 frames
		//
		ret_bytes = lame_encode_flush(gfp, mp3Buffer, sizeof(mp3Buffer) );
		if( ret_bytes > 0 )
		{
			//
			// writes this frames to out file
			//
			ret_write = fwrite( mp3Buffer, 1, ret_bytes, pFileOut );
			if( ret_write != ret_bytes )
			{
				qWarning() << "[SoundManager::lameMp3] - Error saving flush data to " << QString(strFileOut);
				return false;
			}
		}
		//
		// Free the internal data structures
		//
		lame_close(gfp);

		return true;
#endif

	}

/*!
	Init OpenAl.
	The context and device is create and initialized. The device chosen is always the first on the list.

	The path for the samples is also initialized here, the default value is "../samples/".
	This path can be changed by sending other path in \a pathSamples.

	Returns true if openAl was initialized, if the OpenAl is already initialized the function return false;
*/
	bool SoundManager::initOpenAl(int sourcePoolSize)
	{
		qDebug() << "[SoundManager::initOpenAl]";
		if( !isInitAl )
		{
#ifdef _WIN32
			ALFWInit();
			qDebug() << "[SoundManager::initOpenAl] - ALFWInit()";

			ALDeviceList *pDeviceList = NULL;
			ALCcontext *pContext = NULL;
			_pDevice = NULL;
			ALboolean bReturn = AL_FALSE;

			//
			// Create a new OpenAL Device
			//
			pDeviceList = new ALDeviceList();
			ALint numDevices = pDeviceList->GetNumDevices();
			qDebug() << "[SoundManager::initOpenAl] - numDevices =" << QString::number(numDevices);
			qDebug() << "[SoundManager::initOpenAl] - Default DeviceName:" << QString( alcGetString( NULL, ALC_DEFAULT_DEVICE_SPECIFIER ) );

			if( pDeviceList && pDeviceList->GetNumDevices() )
			{
				_pDevice = alcOpenDevice( NULL );  //Gets de default
				if( _pDevice )
				{
					pContext = alcCreateContext( _pDevice, NULL );
					if( pContext )
					{
						//
						// Make the new context the Current OpenAL Context
						//
						alcMakeContextCurrent( pContext );
						bReturn = AL_TRUE;
					}
					else
					{
						qDebug() << "[SoundManager::initOpenAl] -Can't create the context for device";
						alcCloseDevice( _pDevice );
					}
				}
			}
			delete pDeviceList;

			if( !bReturn )
			{
				ALFWShutdown();
				_lastError = CS_INIT_OPENAL;
				qDebug() << "[SoundManager::initOpenAl] -Openal init failed";
				return false;
			}
			else
			{
				isInitAl = true;
			}
#else
			ALCcontext	*pContext = NULL;
			_pDevice = NULL;
			//
			// Create a new OpenAL Device
			// Pass NULL to specify the system’s default output device
			//
			_pDevice = alcOpenDevice(NULL);
			if (_pDevice != NULL)
			{
				//
				// Create a new OpenAL Context
				// The new context will render to the OpenAL Device just created
				//
				pContext = alcCreateContext(_pDevice, 0);
				if (pContext != NULL)
				{
					//
					// Make the new context the Current OpenAL Context
					//
					alcMakeContextCurrent(pContext);
					isInitAl = true;
				}
				else
				{
					qWarning() << "[SoundManager::initOpenAl] - Can't create the context for device";
					_lastError = CS_INIT_OPENAL;
					qWarning() << "[SoundManager::initOpenAl] - Openal init failed";
					return false;
				}
			}
			else
			{
				qWarning() << "[SoundManager::initOpenAl] - Can't create a new OpenAL Device]";
				return false;
			}
#endif

			if (isInitAl)
			{
				//
				// Create Source Pool
				//
				_sourcePool = new SourcePool( sourcePoolSize );

				qDebug() << "[SoundManager::initOpenAl] - openal initialized successful";
				return true;
			}
		}
		else
		{
			qWarning() << "[SoundManager::initOpenAl] - openal already initialized";
		}

		return false;
	}

/*!
	Init functions for Ogg and vorbis files.
	The openAL must be initialized before ogg. Almost is not the returns an error.
	This function need 3 dll, "libvorbisfile.dll", "vorbis.dll", "ogg.dll", and it must be all in the folder of executable.

	Returns true if Ogg & Vorbis where initialized, otherwise false.
	If the ogg is already initialized the function return false;
*/
	bool SoundManager::initOgg()
	{
		qDebug() << "[SoundManager::initOgg()]";
		if( !isInitAl )
		{
			qDebug() << "[SoundManager::initOgg] - openal is not initialized";
			_lastError = CS_OPENAL_NOT_INIT;
			return false;
		}

		if( isInitOgg )
		{
			qDebug() << "[SoundManager::initOgg] - ogg already initialized";
			_lastError = CS_NO_ERROR;
			return true;
		}
#ifdef _WIN32
		//
		// Try and load Vorbis DLLs (VorbisFile.dll will load ogg.dll and vorbis.dll) */
		//
		_g_hVorbisFileDLL = LoadLibraryA("libvorbisfile.dll");
		if( _g_hVorbisFileDLL )
		{
			fn_ov_clear = (LPOVCLEAR)GetProcAddress(_g_hVorbisFileDLL, "ov_clear");
			fn_ov_read = (LPOVREAD)GetProcAddress(_g_hVorbisFileDLL, "ov_read");
			fn_ov_pcm_total = (LPOVPCMTOTAL)GetProcAddress(_g_hVorbisFileDLL, "ov_pcm_total");
			fn_ov_info = (LPOVINFO)GetProcAddress(_g_hVorbisFileDLL, "ov_info");
			fn_ov_comment = (LPOVCOMMENT)GetProcAddress(_g_hVorbisFileDLL, "ov_comment");
			fn_ov_open_callbacks = (LPOVOPENCALLBACKS)GetProcAddress(_g_hVorbisFileDLL, "ov_open_callbacks");
			isInitOgg = true;
			_lastError = CS_NO_ERROR;
			qDebug() << "[SoundManager::initOgg] - ogg initialized successful";
			return true;
		}
		qDebug() << "[SoundManager::initOgg] - libvorbisfile.dll not found";
		_lastError = CS_MISSING_VORBISDLL;
#else
		qDebug() << "[SoundManager::initOgg] -   FALTA IMPLEMENTAR";
#endif
		return false;
	}

/*!
	Function to copy a sound into a new. The name of the new sound \a newSoundName must be different from the name of the sound to copy \a soundNameToCopy.
	There can not be a sound with the same name as the new sound.

	Returns false if is the name of the new sound already exists or is the name of the sound to copy does not exist.
*/
	bool SoundManager::copySound(const QString soundNameToCopy, const QString newSoundName)
	{
		qDebug() << "[SoundManager::copySound] - Copy:" << soundNameToCopy << "New:" << newSoundName;
		if( checkSoundName(soundNameToCopy) )
		{
			if( !checkSoundName(newSoundName) )
			{
				//
				// COPY
				//
				SoundBase* s;
				if( dynamic_cast<Sample*>(_soundList[soundNameToCopy]) != 0 )
					s = new Sample( *((Sample*)(_soundList[soundNameToCopy])), newSoundName );
				else
					s = new Sound( *((Sound*)(_soundList[soundNameToCopy])), newSoundName );
				//
				// Insert new into sound list
				//
				_soundList.insert( std::make_pair(newSoundName, s));
				_lastError = CS_NO_ERROR;
				//
				// Initialize conections for new sound
				//
				s->connectToSoundManager();

				return true;
			}
			else
			{
				qDebug() << "[SoundManager::copySound] -  is already used";
				_lastError = CS_NAME_ALREADY_USED;
				return false;
			}
		}
		else
		{
			qDebug() << "[SoundManager::copySound] - don't exist to copySound";
			_lastError = CS_SOUND_UNKNOW;
			return false;
		}
	}

/*!
	Create a new sound with the name \a soundName with a melody with the following attributes \a tempo, \a instrument and \a compass.
	The sound create is a instance of Sound and not of SoundBase, so it can change the note list.
	The signals of the sound are also initialized here.

	Returns NULL if the name of sound is already on list, otherwise the new sound.
*/
	Sound* SoundManager::createSound(const QString soundName, TempoType tempo,
					EnumInstrument instrument, CompassType compass)
	{
		qDebug() << "[SoundManager::createSound] Name:" << soundName << "Tempo:" << tempo;
		if( checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::createSound]" << soundName << "already exist.";
			_lastError = CS_NAME_ALREADY_USED;
			return NULL;
		}
		//
		// Create sound
		//
		Sound* s;
		s = new Sound(soundName, tempo);
		//
		// Add melody
		//
		s->addMelody(instrument, compass);
		//
		// Insert into sound list
		//
		_soundList.insert( std::make_pair(soundName, s));

		qDebug() << "[SoundManager::createSound]" << soundName << " added to sound list.";
		_lastError = CS_NO_ERROR;
		//
		// Initialize conections for new sound
		//
		s->connectToSoundManager();

		return s;
	}

	/*!
		Create a new sound with the name \a soundName with a melody with the following attributes \a tempo, \a instrument and \a compass.
		The sound create is a instance of Sound and not of SoundBase, so it can change the note list.
		The signals of the sound are also initialized here.

		Returns NULL if the name of sound is already on list, otherwise the new sound.
	*/
		Music* SoundManager::createMusic(const QString soundName, TempoType tempo, EnumInstrument instrument)
		{
			qDebug() << "[SoundManager::createMusic] Name:" << soundName << "Tempo:" << tempo;
			if( checkSoundName(soundName) )
			{
				qDebug() << "[SoundManager::createMusic]" << soundName << "already exist.";
				_lastError = CS_NAME_ALREADY_USED;
				return NULL;
			}
			//
			// Create sound
			//
			Music* s = new Music(soundName);
			//
			// set info
			//
			s->setInstrument(instrument);
			s->setTempo(tempo);
			//
			// Insert into sound list
			//
			_soundList.insert( std::make_pair(soundName, s));

			qDebug() << "[SoundManager::createMusic]" << soundName << " added to sound list.";
			_lastError = CS_NO_ERROR;
			//
			// Initialize conections for new sound
			//
			s->connectToSoundManager();

			return s;
		}

/*!
	Changes, adds, or removes a rythm of a sound. The sound has to be a sound from the class Sound (xml files).

	It's possible to have a type of rhythm for each instrument, that why it only changes the rythm of \a instrument.

	The number of different type of rhythms is 5, only the beatbox has 10.
	To remove a rhythm the value of new \a id (rythm) must be 0.

	The compass also can be different from the music, the default value is 4/4.
*/
	bool SoundManager::editRythms(const QString soundName, EnumInstrument instrument, int id, CompassType compass)
	{
		qDebug() << "[SoundManager::editRythms]" << soundName << ", " << instrument << "," <<  id + "," << compass;
		if(!checkSoundName(soundName)){
			qDebug() << "[SoundManager::editRythms]" << soundName + "don't exist to editRythms";
			return false;
		}

		if( dynamic_cast<Sound*>(_soundList[soundName]) == 0 )
		{
			qDebug() << "[SoundManager::editRythms]" << soundName << " is not a sound with notes";
			_lastError = CS_IS_NOT_XMLSOUND;
			return false;
		}
		//
		// Change the rythm
		//
		bool result = ((Sound*)(_soundList[soundName]))->changeRhythm(instrument, id);
		_lastError = _soundList[soundName]->getLastError();
		return result;
	}

/*!
	Play the sound \a soundName.

	The sound can be play in loop by setting \a loop to true.

	Returns false if there was a error, otherwise true.
*/
	bool SoundManager::playSound(const QString soundName, bool loop, bool blockSignals)
	{
		qDebug() << "[SoundManager::playSound]" << soundName << ", " << loop;
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::playSound]" << soundName << " don't exist to playSound";
			return false;
		}
		//
		// PLAY
		//
		bool result = _soundList[soundName]->playSound(loop, blockSignals);
		_lastError = _soundList[soundName]->getLastError();
		return result;
	}


/*!
	Stops all the sounds on the list of sound.
*/
	bool SoundManager::stopAllSound()
	{
		qDebug() << "[SoundManager::stopAllSound]";
		SoundList::iterator it;
		for( it = _soundList.begin(); it != _soundList.end(); it++ )
		{
			it->second->stopSound();
		}
		return true;
	}

/*!
	Stop the sound \a soundName.

	Returns true if sound was stopped, otherwise false.
*/
	bool SoundManager::stopSound(const QString soundName)
	{
		qDebug() << "[SoundManager::stopSound]" << soundName ;
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::stopSound]" << soundName << "don't exist to stopSound";
			return false;
		}
		if( isSoundStopped(soundName) )
		{
			qDebug() << "[SoundManager::stopSound]" << soundName << "is too stopped";
			return false;
		}
		//
		// STOP
		//
		bool result = _soundList[soundName]->stopSound();
		_lastError = _soundList[soundName]->getLastError();
		return result;
	}

/*!
	Pause the sound \a soundName.

	Returns true if sound was stopped, otherwise false.
*/
	bool SoundManager::pauseSound(const QString soundName)
	{
		qDebug() << "[SoundManager::pauseSound]" << soundName;
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::pauseSound]" << soundName << "don't exist to pauseSound";
			return false;
		}
		//
		// Pause
		//
		bool result = _soundList[soundName]->pauseSound();
		_lastError = _soundList[soundName]->getLastError();
		return result;
	}

/*!
	Play \a melody of sound \a soundName.
	The sound has to be a sound of the class Sound (xml files).

	Returns true if melody was started, otherwise false.
*/
	bool SoundManager::playMelodyOfSound(const QString soundName, int melody)
	{
		qDebug() << "[SoundManager::playMelodyOfSound]" << soundName << ", " + melody;
		if(!checkSoundName(soundName)){
			return false;
		}
		if( dynamic_cast<Sound*>(_soundList[soundName]) == 0 )
		{
			_lastError = CS_IS_NOT_XMLSOUND;
			return false;
		}
		//
		// PLAY melody
		//
		bool value = ((Sound*)(_soundList[soundName]))->playSound(false, melody);
		_lastError = _soundList[soundName]->getLastError();
		return value;
	}

/*!
	Creates and loads a sound to the sound list.

	Loads from the file \a fileName and is saved with the name \a soundName.

	If the file is a xml the sound who has create is an instance of Sound, if the file is wav, the sound is an
	instance of Sample and if the file is an ogg, the sound is an instance of Stream. The file is loaded on
	memory unless if is an ogg file, because the ogg file is play in streaming.

	Before load any file the openal must be initialized and if the file is ogg the ogg must be initialized. If the
	file is an ogg even if the file does not exist or is not valid than the sound will be created and will not give error.

	If \a toOverride is true, even if sound already exists it is loaded again.
	If \a connectSound is false, the signal of the sound will not be initialized.

	Returns true if it was successful, otherwise false.
*/
	bool SoundManager::load(const QString filename, const QString soundName, bool toOverride, bool connectSound)
        {
                if( checkSoundName(soundName) && !toOverride )
                {
			return true;
                }
		//
		// Verifies if files exists
		//
		QString filenamePath = filename;
		QFile f;
		f.setFileName( filenamePath );
		if( !f.exists() )
                {
			filenamePath = samplePath(filenamePath);
			if(filenamePath.isEmpty())
                        {
				qDebug() << "[SoundManager::load]" << filenamePath << " doesn't exist";
				_lastError = CS_FILE_NOT_FOUND;
				return false;
			}
		}

		qDebug() << "[SoundManager::load]" << filenamePath <<  ", " << soundName;
		QString newSoundName;
		//
		// If not given a sound name uses the file name as the sound name
		//
		newSoundName = ( soundName == "" ) ? filenamePath : soundName;
		//
		// Check if openAL is initialized
		//
		if( !isInitAl )
		{
			qDebug() << "[SoundManager::load]Open Al is not initialized";
			_lastError = CS_OPENAL_NOT_INIT;
			return false;
		}
		//
		// Creates the new sound
                //
		SoundBase* s;
		bool result;
		if ( filenamePath. contains( ".wav", Qt::CaseInsensitive ) )
                {
			s = new Sample(newSoundName);
		}
		else if( filenamePath. contains( ".xml", Qt::CaseInsensitive ) )
                {
			s = new Sound(newSoundName);
		}
		else if( filenamePath. contains( ".ogg", Qt::CaseInsensitive ) )
                {
			//
			// Check if Ogg & Vorbis are initialized
			//
			if( !isInitOgg )
                        {
				qDebug() << "[SoundManager::load]Ogg is not initialized";
				_lastError = CS_OGG_NOT_INIT;
				return false;
			}
			s = new Stream(newSoundName);
		}
		else
                {
			_lastError = CS_EXTENSION_UNKNOW;
			return false;
		}
		//
		// Loads sound
                //
		result = s->load( filenamePath );
		if( result )
                {
			_lastError = CS_NO_ERROR;
			if( checkSoundName( newSoundName ) )
			{
				qDebug() << "[SoundManager::load]" << newSoundName << " already exists so it is release";
				releaseSound( newSoundName );
			}
			//
			// Adds to sound list
			//
			_soundList.insert( std::make_pair(newSoundName, s) );
			if( connectSound )
			{
				//
				// Initialize connections
				//
				s->connectToSoundManager();
			}
		}
		else
		{
			qDebug() << "[SoundManager::load] Error loading sound.";
			_lastError = s->getLastError();
			delete(s);
		}
		if( !result )
		{
			qDebug() << "[SoundManager::load] '" + filenamePath + "' - failed";
		}
		return result;
	}

/**
	Release the sound \a soundName.
	Before releasing a sound, he's stopped.

	Returns true if sound was released, otherwise false.
*/
	bool SoundManager::releaseSound(const QString soundName)
	{
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::releaseSound]" << soundName << "don't exist to releaseSound";
			return false;
		}
		//
		// STOP sound
		//
		// stopSound( soundName ); // It's done in the destructor.
		//
		// DELETE sound
		//
		delete(_soundList[soundName]);
		_soundList[soundName] = 0;
		_soundList.erase(soundName);

		_lastError = CS_NO_ERROR;

		qDebug() << "[SoundManager::releaseSound] Released:" << soundName;
		return true;
	}

/*!
	Release all sound.

	Returns always true.
*/
	bool SoundManager::releaseAllSound()
	{
		qDebug() << "[SoundManager::releaseAllSound]";
		SoundList::iterator it;
		//
		// Delete sounds
		//
		for( it = _soundList.begin(); it != _soundList.end(); it++ )
		{
			delete(it->second);
			it->second = 0;
		}
		//
		// Clear sound list
		//
		_soundList.clear();

		_lastError = CS_NO_ERROR;
		return true;
	}

/*!
	Function to know if the sound \a soundName is playing or not.

	Returns false if the sound don't exist or if the sound is not playing, otherwise true.
*/
	bool SoundManager::isSoundPlaying(const QString soundName)
	{
		qDebug() << "[SoundManager::isSoundPlaying]" << soundName;
		if( checkSoundName(soundName) )
		{
			bool result = _soundList[soundName]->isPlaying();
			qDebug() << "[SoundManager::isSoundPlaying]"  << soundName << "isPlaying" << result;
			return result;
		}
		else
		{
			_lastError = CS_SOUND_UNKNOW;
			qDebug() << "[SoundManager::isSoundPlaying]"  << soundName << " don't exist to isSoundPlaying";
			return false;
		}
	}

/*!
	Function to know if the sound \a soundName is paused or not.

	Returns false if the sound don't exist or if the sound is not paused, otherwise true.
*/
	bool SoundManager::isSoundPaused(const QString soundName)
	{
		qDebug() <<"[SoundManager::isSoundPaused]" << soundName;
		if(checkSoundName(soundName))
		{
			bool result = _soundList[soundName]->isPaused();
			qDebug() <<"[SoundManager::isSoundPaused]" << soundName << "isPaused" << result;
			return result;
		}
		else
		{
			_lastError = CS_SOUND_UNKNOW;
			qDebug() <<"[SoundManager::isSoundPaused]" << soundName << " don't exist to isSoundPaused";
			return false;
		}
	}

/*!
	Function to know if the sound \a soundName is stoped or not.

	Returns false if the sound don't exist or if the sound is not stopped, otherwise true.
*/
	bool SoundManager::isSoundStopped(const QString soundName)
	{
		qDebug() << "[SoundManager::isSoundStopped]" << soundName;
		if(checkSoundName(soundName))
		{
			bool result = _soundList[soundName]->isStopped();
			qDebug() << "[SoundManager::isSoundStopped]" << soundName << " isSoundStopped " + result;
			return result;
		}
		else
		{
			_lastError = CS_SOUND_UNKNOW;
			qDebug() << "[SoundManager::isSoundStopped]" << soundName << " don't exist to isSoundStopped";
			return false;
		}
	}

/*!
	Function to know if a sound is empty or not. This function is not implemented for wav and ogg sound.

	Returns false if the sound don't exist or if the sound is not empty, otherwise true.
*/
	bool SoundManager::isSoundEmpty(const QString soundName)
	{
		qDebug() <<"SoundManager::isSoundEmpty(" << soundName;
		if(checkSoundName(soundName))
		{
			bool result = _soundList[soundName]->isEmpty();
			qDebug() << soundName << " isSoundEmpty "  << result;
			return result;
		}
		else
		{
			_lastError = CS_SOUND_UNKNOW;
			qDebug() <<soundName  <<  "don't exist to isSoundEmpty";
			return false;
		}
	}

/*!
	Compares two sounds \a soundOne and \a soundTwo.

	This function is not implemented for wav and ogg sound.

	Returns false if one of the sounds don't exist or if the sounds are not iqual.
*/
	bool  SoundManager::compareSound(const QString soundOne, const QString soundTwo)
	{
		qDebug() <<"SoundManager::compareSound(" << soundOne << ", " << soundTwo;
		if( !checkSoundName(soundOne) )
		{
			qDebug() <<soundOne  <<  "don't exist to compareSound";
			return false;
		}
		if( !checkSoundName(soundTwo) )
		{
			qDebug() <<soundTwo  <<  "don't exist to compareSound";
			return false;
		}
		else
		{
			bool result = _soundList[soundOne]->compareSound( _soundList[soundTwo] );
			if( !result )
			{
				_lastError = _soundList[soundOne]->getLastError();
			}
			else
			{
				_lastError = CS_NO_ERROR;
			}
			return result;
		}
	}

/*!
	Compare two melody, \a firstMelody and \a secondMelody of the sound \a soundName.

	This function is not implemented for wav and ogg sound.

	Returns false if one of the sounds don't exist or if the melody are not igual.
*/
	int  SoundManager::compareMelody( const QString soundName, int firstMelody, int secondMelody )
	{
		qDebug() << "[SoundManager::compareMelody] " << soundName << "," << firstMelody << "," << secondMelody;
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::compareMelody] " << soundName << " doesn't exist to compareMelody";
			return false;
		}
		else
		{
			if( dynamic_cast<Sound*>(_soundList[soundName]) != 0 )
			{
				int value = ((Sound*)(_soundList[soundName]))->compareMelody(firstMelody, secondMelody);
				if( value < 0 )
					_lastError = _soundList[soundName]->getLastError();
				else
					_lastError = CS_NO_ERROR;
				return value;
			}
			else
			{
				qWarning() << "[SoundManager::compareMelody]" << soundName << " is not a sound with notes.";
				_lastError = CS_IS_NOT_XMLSOUND;
				return false;
			}
		}
	}

/*
	Saves the sound \a soundname into a file named \a filename.

	To overwrite existent file, \a overwrite must be true.

	This function is not implemented for ogg sound.

	Returns true if sound was saved, otherwise false.
*/
	bool SoundManager::save(const QString soundName, const QString filename, bool overwrite)
	{
		qDebug() << "[SoundManager::save]" << soundName << "," << filename << "," << overwrite;
		//
		// Check if file exists
		//
		QFile f( filename + ".wav" );
		if( overwrite == false && f.exists() )
		{
			qDebug() << "[SoundManager::save]" << filename << " already exist";
			_lastError = CS_FILE_EXISTS;
			return false;
		}
		//
		// SAVE
		//
		if( checkSoundName(soundName) )
		{
			return _soundList[soundName]->save( filename );
		}
		else
		{
			qDebug() << "[SoundManager::save]" <<  soundName + " don't exist to saveWav";
			return false;
		}
	}

/*
	Saves the meldoy \a melody of the sound \a soundname into wav file named \a filename.

	To overwrite existent file, \a overwrite must be true.

	Returns true if sound was saved, otherwise false.
*/
	bool SoundManager::saveMelodyWav(const QString soundName, int melody, QString filename, bool overwrite)
	{
		qDebug() << "[SoundManager::saveMelodyWav]" << soundName << "," << melody << ", " << filename << "," << overwrite;
		//
		// Chck if file exists
		//
		QFile f( filename + ".wav" );
		if( overwrite == false && f.exists() )
		{
			qDebug() << "[SoundManager::saveMelodyWav]" << filename << " already exist.";
			_lastError = CS_FILE_EXISTS;
			return false;
		}
		//
		// SAVE
		//
		if( checkSoundName(soundName) )
		{
			if( dynamic_cast<Sound*>(_soundList[soundName]) != 0 )
			{
				return ((Sound*)(_soundList[soundName]))->saveWav(filename);
			}
			else
			{
				qDebug() << "[SoundManager::saveMelodyWav]" << soundName << " is not a sound with notes";
				_lastError = CS_IS_NOT_XMLSOUND;
				return false;
			}
		}
		else
		{
			qDebug() << "[SoundManager::saveMelodyWav]" << soundName << " don't exist to saveMelodyWav";
			return false;
		}
	}

/*!
	Function to save a sound (\a soundName) into mp3 file (\a filename).

	To do the encode, the sound is first saved into a wav file. To keep this wav file after
	the encode \a deleteWav must be set to false.

	In the \a minimumRate is set the minimum rate for the mp3 sound.

	If the file already exists, and is not to replace \a overwrite must be false.

	This function is not implemented for ogg sound.

	Returns true if file was saved, otherwise false.
*/
	bool SoundManager::saveMp3(const QString soundName, const QString filename, int minimumRate,  bool deleteWav, bool overwrite)
	{
		qDebug() << "[SoundManager::saveMp3]" << soundName << "," << filename << "," << minimumRate << "," << deleteWav << "," << overwrite;
		//
		// Check if file already exists
		//
		QFile f( filename + ".mp3" );
		if( overwrite == false && f.exists() )
		{
			qDebug() << "[SoundManager::saveMp3]" << filename << " already exist";
			_lastError = CS_FILE_EXISTS;
			return false;
		}
		//
		// Check if sound exists
		//
		if( !checkSoundName(soundName) )
		{
			qDebug()  << "[SoundManager::saveMp3]" << soundName << " doesn't exist to saveMp3";
			return false;
		}
		//
		// SAVE MP3
		//
		return _soundList[soundName]->saveMp3(filename, minimumRate, deleteWav);
	}

/*!
	Returns the percent of the sound playing or 0.0 if the sound doesn't exist or is stopped.

	This function is not implemented for wav and ogg sound.
*/
	float SoundManager::percentPlay(const QString soundName)
	{
		//
		// Check if sound exists
		//
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::percentPlay] " << soundName << " dones't exist to percentPlay";
			return 0.0;
		}
		//
		// Get percent play for the sound
		//
		return _soundList[soundName]->percentPlay();
	}

/*
	Returns the last error occurred during a operation.
*/
	CnotiErrorSound SoundManager::getLastError()
	{
		return _lastError;
	}

/*!
	Function to loaded a note wav (sample).

	Requires the note information: \a height, \a octave, \a duration, \a tempo
	and \a instrument. This infomation is used to contruct the filename of the
	file to load.

	To load the note, openAL must be initialized.

	Returns true if note (sample) was loaded, otherwise false.
*/
	bool SoundManager::loadSampleNote(NoteType height, int octave, DurationType duration, TempoType tempo, EnumInstrument instrument)
	{
		qDebug() << "[SoundManager::loadSampleNote] Height:" << height << "Duration:" << duration << "Tempo:" << tempo << "Intrument" << instrument;
		QString filename = nameNote( instrument, tempo, duration, octave, height );

		return load(filename, filename);
	}

/*!
	Function to release a note (sample).

	Requires the note information: \a height, \a octave, \a duration, \a tempo
	and \a instrument. This infomation is used to contruct the filename of the
	file to load.

	To load the note, openAL must be initialized.

	Returns true if note (sample) was released, otherwise false.
*/
	void SoundManager::releaseSampleNote(NoteType height, int octave, DurationType duration, TempoType tempo, EnumInstrument instrument)
	{
		QString filename = nameNote( instrument, tempo, duration, octave, height );
		releaseSound( filename );
	}

/*!
	Loades the 7 notes higher of one octave (\a octave).

	Requires the note information: \a duration, \a tempo and \a instrument.
	This infomation is used to contruct the filename of the file to load.

	To load the notes, openAL must be initialized.

	Returns true if all the notes where loaded, otherwise false.
	*/
	bool SoundManager::loadSamplePrincipalNotes(int octave, DurationType duration, TempoType tempo, EnumInstrument instrument)
	{
		_lastError = CS_NO_ERROR;
		bool result = true;

		QList<int> noteList; // Using note type gives error adding type to list
		noteList << (int)DO << (int)RE << (int)MI << (int)FA << (int)SOL << (int)LA << (int)SI;
		//
		// Load the 7 notes
		//
		int i = 0;
		while( i < noteList.size() )
		{

			if( !loadSampleNote( (CnotiAudio::NoteType)noteList[i], octave, duration, tempo, instrument ) )
			{
				result = false;
				break;
			}
			i++;
		}
		//
		// Load the pause
		//
		QString pause = pauseName + QString::number(tempo) + "_" + QString::number(duration) + ".wav";
		result = load(pause, pause, false) && result;

		if( !result )
		{
			_lastError = CS_FILE_ERROR;
			//
			// Releases the loaded notes
			// At this point i is always with one value up from the start value we want
			//
			while( --i >= 0 )
			{
				releaseSampleNote( (CnotiAudio::NoteType)noteList[i], octave, duration, tempo, instrument );
			}
			releaseSound( pause );
		}

		return result;
	}

	/*!
	  Loads all the samples for an instrument witha a given tempo.
	*/
	bool SoundManager::loadInstrumentSamples(EnumInstrument instrument, TempoType tempo)
	{
		bool loaded;
		loaded = loadSamplePrincipalNotes(3, CnotiAudio::CROTCHET, tempo, instrument);
		loaded = loadSamplePrincipalNotes(3, CnotiAudio::MINIM, tempo, instrument) && loaded;
		loaded = loadSamplePrincipalNotes(3, CnotiAudio::MINIM_DOTTED, tempo, instrument) && loaded;
		loaded = loadSamplePrincipalNotes(3, CnotiAudio::SEMIBREVE, tempo, instrument) && loaded;
		loaded = loadSampleNote(CnotiAudio::DO, 4, CnotiAudio::CROTCHET, tempo, instrument) && loaded;
		loaded = loadSampleNote(CnotiAudio::DO, 4, CnotiAudio::MINIM, tempo, instrument) && loaded;
		loaded = loadSampleNote(CnotiAudio::DO, 4, CnotiAudio::MINIM_DOTTED, tempo, instrument) && loaded;
		loaded = loadSampleNote(CnotiAudio::DO, 4, CnotiAudio::SEMIBREVE, tempo, instrument) && loaded;

		return loaded;
	}

/*!
	Function to loaded the samples for the rhytms of one \a instrument.

	Are loaded the samples with the tempo \a tempo.

	Return true if all the samples where loaded, otherwise false.
*/
	bool SoundManager::loadRhythms(EnumInstrument instrument, TempoType tempo)
	{
		_lastError = CS_NO_ERROR;
		qDebug() << "[SoundManager::loadRhythms]" << instrument << "," << tempo;
		bool value = true;
		int max = 0;
		CnotiAudio::DurationType noteDuration;
		//
		// Gets correct information about the number o rythm to laod and the note duration
		//
		if( instrument == CnotiAudio::RHYTHM_INST_BEAT_BOX )
		{
			max = CS_NUMBERRYTHM_BEATBOX;
			noteDuration = BREVE;
		}
		else
		{
			max = CS_NUMBERRYTHM;
			noteDuration = SEMIBREVE;
		}
		//
		// Load samples
		//
		for( int i=0; i < max; i++ )
		{
			value = loadSampleNote((NoteType)(i), 3, noteDuration, tempo, instrument) && value;
		}

		if( !value )
		{
			//
			// Releases the samples
			//
			_lastError = CS_FILE_ERROR;
			releaseSamplesInstrument( instrument );
			qWarning() << "[SoundManager::loadRhythms] Error loading samples for a ryhtm";
		}
		return value;
	}

	/*!
		Function to loaded a rhythm wav (sample).

		Requires the note information: \a height, \a octave, \a duration, \a tempo
		and \a instrument. This infomation is used to contruct the filename of the
		file to load.

		To load the note, openAL must be initialized.

		Returns true if note (sample) was loaded, otherwise false.
	*/
	bool SoundManager::loadRhythmSample(EnumRhythmInstrument instrument, TempoType tempo, EnumRhythmVariation variation)
	{
		qDebug() << "[SoundManager::loadSampleRhythm] Height:" << variation << "Tempo:" << tempo << "Rhythmic intrument" << instrument;
		QString filename = rhythmName( instrument, tempo, variation );
		return load(filename, filename, false, false);
	}

/*!
	Release all the samples (wav) of an \a instrument.

	Returns true if all where released, otherwise false.
*/
	bool SoundManager::releaseSamplesInstrument(EnumInstrument instrument)
	{
		_lastError = CS_FILE_ERROR;
		QString mask = QString::number((int)(instrument)) + "_???_??*.wav";	// So it doesn't unload rhythms
		return releaseSamplesMask( mask );
	}

/*!
	Release all samples wav with a certain \a mask.

	Returns true if no exceptions occurred, otherwise false.
*/
	bool SoundManager::releaseSamplesMask( QString mask )
	{
		_lastError = CS_FILE_ERROR;
		// Get the name of the sound to be released
		QStringList deleteList;
		SoundList::iterator it;
		try
		{
			QRegExp rx(mask);
			rx.setPatternSyntax(QRegExp::Wildcard);
			for(it = _soundList.begin(); it != _soundList.end(); it++ )
			{
				if( it->first.contains( rx ) )
				{
					deleteList << it->first;
				}
			}
		}
		catch (...)
		{
			qWarning() << "[SoundManager::releaseSamplesMask] Exception occured on releasing samples";
			return false;
		}

		// Release sounds
		QStringListIterator strIt(deleteList);
		while (strIt.hasNext())
		{
			releaseSound(strIt.next());
		}

		return true;
	}

/*!
	Plays a single note in \a instument, with a \a duration, a \an height, a \an octave
	and an \a intensity.

	Retunrs true if note played, otherwise false.
*/
	bool SoundManager::playNote( EnumInstrument instrument, TempoType tempo, DurationType duration, NoteType height, int octave, float intensity )
	{
		_lastError = CS_FILE_ERROR;
		QString filename  = nameNote(instrument, tempo, duration, octave, height);
		//
		// Check if note exists (is loaded)
		//
		if( !checkSoundName(filename) )
		{
			qDebug() << "[SoundManager::playNote]" << filename << " don't exist to playNote";
			return false;
		}

		_soundList[filename]->setVolume( intensity );
		return _soundList[filename]->playSound();
	}

/*!
	Change the intensity of \a soundName to \a intensity.

	The value of intensity can be change between 0 and 1.

	Returns false if the sound doesn't exist.
*/
	bool SoundManager::setSoundIntensity(const QString soundName, float intensity)
	{
		_lastError = CS_FILE_ERROR;
		//
		// Check if sound exists
		//
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::setSoundIntensity]" << soundName << " don't exist to setIntensitySound";
			_lastError = CS_SOUND_UNKNOW;
			return false;
		}

		_soundList[soundName]->setVolume(intensity);
		qDebug() << "[SoundManager::setSoundIntensity]" << soundName << "Intensity:" << intensity;

		return true;
	}

/*!
	Change the intensity of all the sound that in their names as \a mask.
	Returns true no error occureed, otherwisse false.
*/
	bool SoundManager::setSoundIntensityMask( const QString mask, float intensity )
	{
		_lastError = CS_FILE_ERROR;
		SoundList::iterator it;
		try
		{
			for( it = _soundList.begin(); it != _soundList.end(); it++ )
			{
				if( it->first.contains( mask ) )
				{
					setSoundIntensity( it->first, intensity );
				}
			}
		}
		catch (...)
		{
			return false;
		}
		return true;
	}

/*!
	Change the intensity of a sound of all the melodies played by \a intrument
	to \a intensity in the sound identified by \a soundName.

	Returns true if no error occurred, otherwise false.
*/
	bool SoundManager::setMelodyIntensity( const QString &soundName, const CnotiAudio::EnumInstrument inst, float intensity )
	{
		_lastError = CS_FILE_ERROR;
		QList<Melody*>::iterator it;
		try
		{
			Sound* s = (Sound*)_soundList[soundName];
			QList<Melody*> melodyList = s->getMelodyList();
			//
			// Checks every melody in the sound
			//
			for( it = melodyList.begin(); it != melodyList.end(); it++ )
			{
				//
				// Check if is the correct instrument
				//
				if((*it)->getInstrument() == inst )
				{
					//
					// Changes volume
					//
					(*it)->setIntensity( intensity );
				}
			}
		}
		catch (...)
		{
			return false;
		}

		return true;
	}

/*
	Returns the intensity of sound named \a soundName.

	If the sound doesn't exist returns -1.
*/
	float SoundManager::getIntensitySound(const QString soundName)
	{
		if( !checkSoundName(soundName) )
		{
			qDebug() << soundName << " don't exist to getIntensitySound";
			return -1;
		}

		return _soundList[soundName]->getIntensity();
	}

/*!
	Changes the intensity of all the sounds to \a intensity.

	The value os intensity can be change between 0 and 1.
*/
	void SoundManager::setIntensity( float intensity )
	{
		SoundList::iterator it;
		for( it = _soundList.begin(); it != _soundList.end(); it++ )
		{
			it->second->setVolume( intensity );
		}
	}


/*!
	Return pointer to the sound identified by \a soundName.

	If the sound is an wav or ogg the function return NULL.
*/
	Sound*  SoundManager::getSound(const QString soundName)
	{
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::getSound]" << soundName << "don't exist to getSound";
			return NULL;
		}

		return ((Sound*)(_soundList[soundName]));
	}

/*!
	Returns the buffer of a note (sample).
*/
	ALuint SoundManager::getBufferFromNote( DurationType duration, NoteType height, int octave,
											TempoType tempo, EnumInstrument instrument)
	{
		//
		// Constrcts the note name
		//
		QString filename = nameNote(instrument, tempo, duration, octave, height );
		//
		// Verifies if sound exists
		//
		if( !checkSoundName(filename) )
		{
			qDebug() << "[SoundManager::getBufferFromNote]" <<  filename << " doesn't exist to getBufferFromNote";
			return 0;
		}

		return ((Sample*)(_soundList[filename]))->getBuffer();
	}

/*!
	Returns the data of \a soundName.
*/
	short* SoundManager::getData(const QString soundName)
	{
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::getData]" << soundName << "doesn't exist to getData";
			return 0;
		}

		return _soundList[soundName]->getData();
	}

/*!
	Returns the size of \a soundName.
*/
	unsigned long SoundManager::getSize(const QString soundName)
	{
		if( !checkSoundName(soundName) )
		{
			qDebug() << "[SoundManager::getSize]" <<  soundName << "doesn't exist to getSize";
			return 0;
		}

		return _soundList[soundName]->getSize();
	}

/*!
	Returns the frequency of \a soundName.
*/
	ALint SoundManager::getFrequency(const QString soundName)
	{
		//qDebug() <<"SoundManager::getFrequency(" << soundName;
		if( checkSoundName(soundName) )
		{
			return _soundList[soundName]->getFrequency();
		}
		else
		{
			qDebug() << "[SoundManager::getFrequency]" << soundName << " doesn't exist to getFrequency";
			return 0;
		}
	}

/*!
	Returns the duration of \a soundName.
*/
	float SoundManager::getDuration(const QString soundName)
	{
		if( checkSoundName(soundName) )
		{
			return _soundList[soundName]->getDuration();
		}
		else
		{
			qDebug() << "[SoundManager::getDuration]" << soundName << " doesn't exist to getDuration";
			return 0;
		}
	}

/*!
	Returns the name of one note.

	The note name is contructed using it's information: \a instrument, \a tempo, \a duration,
	\a octave and \a height
*/
	QString SoundManager::nameNote( EnumInstrument instrument, TempoType tempo,
									DurationType duration, int octave, NoteType height )
	{
		QString filename;
		if( height == PAUSE )
		{
			filename = pauseName + QString::number(tempo) + "_" + QString::number(duration) + ".wav";
		}
		else
		{
			filename  = QString::number(instrument) + "_" + QString::number(tempo) + "_";
			filename += QString::number(duration) + "_" + QString::number(octave) + "_";
			filename += QString::number(height) + ".wav";
		}

		return filename;
	}

/*!
	Returns the name of one rhythm.

	The note name is contructed using it's information: \a instrument, \a tempo, \a duration,
	\a octave and \a height
*/
	QString SoundManager::rhythmName(EnumRhythmInstrument instrument, TempoType tempo, EnumRhythmVariation variation)
	{
		QString filename;

		filename  = QString::number(instrument) + "_" + QString::number(tempo) + "_";
		filename += QString::number(variation) + ".wav";

		return filename;
	}

/*!
	Checks if \a soundName is exists.

	A sound exists if the sound was loaded an it was no yet released.

	returns true if sound exists, otherwise false.
*/
	bool SoundManager::checkSoundName(const QString soundName)
	{
		if( _soundList.find(soundName) != _soundList.end() && _soundList[soundName] != 0 )
		{
			_lastError = CS_NO_ERROR;
			return true;
		}

		_lastError = CS_SOUND_UNKNOW;
		return false;
	}

	void SoundManager::addSamplePath(QString path)
	{
		_samplesPath << path;
	}

	void SoundManager::addSamplePaths(QStringList paths)
	{
		QStringListIterator it(paths);
		while(it.hasNext())
		{
			_samplesPath << it.next();
		}
	}

	QString SoundManager::samplePath(QString filename)
	{
		QFile file;
		QStringListIterator it(_samplesPath);
		while(it.hasNext())
		{
			file.setFileName(it.next() + filename);
			if(file.exists())
			{
				return file.fileName();
			}
		}
		return QString();
	}

	/*************
	 *  SOURCES  *
	 *************/
/*!
	Returns a sound source from pool.
*/
	ALuint SoundManager::checkOutSource()
	{
		if(_sourcePool)
		{
			return _sourcePool->checkOut();
		}

		qWarning() << "[SoundManager::checkOutSource] SourcePool is NULL";
		return 0;
	}

/*!
	The sound source \source is returned to the pool.
*/
	void SoundManager::checkInSource( ALuint uiSource )
	{
		if(_sourcePool)
		{
			_sourcePool->checkIn( uiSource );
		}
		else
		{
			qWarning() << "[SoundManager::checkInSource] SourcePool is NULL";
		}
	}

/*!
	Stops the sound being played in the sound source \a uiSource.
*/
	bool SoundManager::stopSound( ALuint uiSource )
	{
		ALenum state;
		alGetSourcei( uiSource, AL_SOURCE_STATE, &state );
		if( state == AL_PLAYING )
		{
			int error = alGetError(); // clear last error
			// Stop source
			alSourceStop(uiSource);
			error = alGetError();
			if( error != AL_NO_ERROR )
			{
				qWarning() << "[SoundManager::stopSound] ERROR stop playing " << error;
				return false;
			}
			return true;
		}
		return false;
	}

	/*********************
	 *  LOG INFORMATION  *
	 *********************/
/*!
	Returns the name of the log file.
*/
	const QString SoundManager::getLogFile()
	{
		return _appName + " - Sound.log";
	}

	/*******************
	 *  SOUND CAPTURE  *
	 *******************/
/*!
	Init the capture process.
*/
	void SoundManager::initCapture()
	{
		_captureThread = new CaptureThread();

		connect( _captureThread, SIGNAL( signalSampleCaptured() ), this, SIGNAL( signalSampleCaptured() ) );
		connect( _captureThread, SIGNAL( signalCaptureEnded() ), this, SIGNAL( signalCaptureStopped() ) );
	}

/*
	Starts capturing sound.

	Note - if capture was not initialized it sends a signal that capture has stopped
*/
	void SoundManager::startCapture( const QString filename )
	{
		_captureThread->startCapture( filename );
	}

/*!
	Stops capturing sound.
*/
	void SoundManager::stopCapture()
	{
		if( _captureThread != NULL )
		{
			_captureThread->stopCapture();
		}
	}

/*!
	Returns the list of devices available to record sound.
*/
	QStringList SoundManager::getCaptureDeviceList()
	{
		return _captureThread->deviceList();
	}

/*!
	Returns the current device used to record sound.
*/
	const QString SoundManager::getCaptureDevice()
	{
		return _captureThread->getDevice();
	}

/*!
	Changes the device used to record sound.
*/
	void SoundManager::changeCaptureDevice( const QString& deviceName )
	{
		return _captureThread->changeDevice( deviceName );
	}

/*!
	NOT IMPLEMENTED.
*/
	int SoundManager::getSizeCapturedBuffer()
	{
		return 0;
	}
/*!
	NOT IMPLEMENTED.
*/
	short* SoundManager::getDataCapturedBuffer()
	{
		return 0;
	}

	QByteArray* SoundManager::getDataCapturedBuffer( int index )
	{
		return _captureThread->getBufferListData( index );
	}
	/**********************
	*  NOTE INFORMATION  *
	**********************/

	QString SoundManager::getDurationTypeText( float duration, int tempo )
	{
		return _noteMisc->getDurationTypeText( duration, tempo );
	}

	int SoundManager::getDurationType( float duration, int tempo )
	{
		return _noteMisc->getDurationType( duration, tempo );
	}

	QString SoundManager::midiNoteToName( int midiValue )
	{
		return _noteMisc->midiNoteToName( midiValue );
	}

	int SoundManager::midiToNote( int midiValue )
	{
		return _noteMisc->midiToNote( midiValue );
	}

	void SoundManager::changeNoteMiscDeviation( float newDeviation )
	{
		return _noteMisc->setDurationDeviation( newDeviation );
	}

	/**********************
	*  STATIC CONVERTERS  *
	**********************/
	TempoType SoundManager::convertStrToTempo(QString tempo)
	{
		return convertIntToTempo(tempo.toInt());
	}

	/*!

	*/
	TempoType SoundManager::convertIntToTempo(int tempo)
	{
		switch(tempo)
		{
		case TEMPO_60:
			return TEMPO_60;
		case TEMPO_120:
			return TEMPO_120;
		case TEMPO_160:
			return TEMPO_160;
		case TEMPO_200:
			return TEMPO_200;
		}
		return TEMPO_UNKNOWN;
	}

	/*!

	*/
	EnumInstrument SoundManager::convertStrToInstrument(QString instrument)
	{
		return convertIntToInstrument(instrument.toInt());
	}

	/*!

	*/
	EnumInstrument SoundManager::convertIntToInstrument(int instrument)
	{
		switch(instrument)
		{
		case PIANO:
			return PIANO;
		case FLUTE:
			return FLUTE;
		case VIOLIN:
			return VIOLIN;
		case XYLOPHONE:
			return XYLOPHONE;
		case TRUMPET:
			return TRUMPET;
//		case ORGAN:
//			return ORGAN;
		}
		return INSTRUMENT_UNKNOWN;
	}

	/*!

	*/
	NoteType SoundManager::convertStrToHeight(QString height)
	{
		return convertIntToHeight(height.toInt());
	}

	/*!

	*/
	NoteType SoundManager::convertIntToHeight(int height)
	{
		switch(height)
		{
		case PAUSE:
			return PAUSE;
		case DO:
			return DO;
		case REb:
			return REb;
		case RE:
			return RE;
		case MIb:
			return MIb;
		case MI:
			return MI;
		case FA:
			return FA;
		case SOLb:
			return SOLb;
		case SOL:
			return SOL;
		case LAb:
			return LAb;
		case LA:
			return LA;
		case SIb:
			return SIb;
		case SI:
			return SI;
		}
		return UNKNOWN_NOTE;
	}

	/*!

	*/
	DurationType SoundManager::convertStrToDuration(QString duration)
	{
		return convertIntToDuration(duration.toInt());
	}

	/*!

	*/
	DurationType SoundManager::convertIntToDuration(int duration)
	{
		switch(duration)
		{
		case LONGA:
			return LONGA;
		case BREVE:
			return BREVE;
		case SEMIBREVE_DOTTED:
			return SEMIBREVE_DOTTED;
		case SEMIBREVE:
			return SEMIBREVE;
		case MINIM_DOTTED:
			return MINIM_DOTTED;
		case MINIM:
			return MINIM;
		case CROTCHET_DOTTED:
			return CROTCHET_DOTTED;
		case CROTCHET:
			return CROTCHET;
		case QUAVER_HALF:
			return QUAVER_HALF;
		case QUAVER:
			return QUAVER;
		case SEMIQUAVER:
			return SEMIQUAVER;
		case DEMISEMIQUAVER:
			return DEMISEMIQUAVER;
		case HEMIDEMISEMIQUAVER:
			return HEMIDEMISEMIQUAVER;
		case SEMIHEMIDEMISEMIQUAVER:
			return SEMIHEMIDEMISEMIQUAVER;
		}
		return UNKNOWN_DURATION;
	}

	/*!

	*/
	EnumOctave SoundManager::convertStrToOctave(QString octave)
	{
		return convertIntToOctave(octave.toInt());
	}

	/*!

	*/
	EnumOctave SoundManager::convertIntToOctave(int octave)
	{
		switch(octave)
		{
		case OCTAVE_C3:
			return OCTAVE_C3;
		case OCTAVE_C4:
			return OCTAVE_C4;
		}
		return OCTAVE_UNKNOWN;
	}

	/*!

	*/
	EnumRhythmInstrument SoundManager::convertStrToRhythmInstrument(QString rhythm)
	{
		return convertIntToRhythmInstrument(rhythm.toInt());
	}

	/*!

	*/
	EnumRhythmInstrument SoundManager::convertIntToRhythmInstrument(int rhythm)
	{
		switch(rhythm)
		{
		case RHYTHM_INST_BASS_DRUM:
			return RHYTHM_INST_BASS_DRUM;
		case RHYTHM_INST_CHINESE_BOX:
			return RHYTHM_INST_CHINESE_BOX;
		case RHYTHM_INST_CONGAS:
			return RHYTHM_INST_CONGAS;
		case RHYTHM_INST_TAMBOURINE:
			return RHYTHM_INST_TAMBOURINE;
		case RHYTHM_INST_TRIANGLE:
			return RHYTHM_INST_TRIANGLE;
		case RHYTHM_INST_BEAT_BOX:
			return RHYTHM_INST_BEAT_BOX;
		}
		return RHYTHM_INST_UNKNOWN;
	}

	EnumRhythmVariation SoundManager::convertStrToRhythmVariation(QString variation)
	{
		return convertIntToRhythmVariation(variation.toInt());
	}

	EnumRhythmVariation SoundManager::convertIntToRhythmVariation(int variation)
	{
		switch(variation)
		{
		case RHYTHM_01:
			return RHYTHM_01;
		case RHYTHM_02:
			return RHYTHM_02;
		case RHYTHM_03:
			return RHYTHM_03;
		case RHYTHM_04:
			return RHYTHM_04;
		case RHYTHM_05:
			return RHYTHM_05;
		case RHYTHM_06:
			return RHYTHM_06;
		case RHYTHM_07:
			return RHYTHM_07;
		case RHYTHM_08:
			return RHYTHM_08;
		case RHYTHM_09:
			return RHYTHM_09;
		case RHYTHM_10:
			return RHYTHM_10;
		}
		return RHYTHM_UNKNOWN;
	}

	/*!

	*/
}

//      bool errorAL = false;
  //      switch(alGetError())
  //  {

  //      case AL_INVALID_NAME:
  //          CnotiLogManager::getSingleton().getLog(soundLog)->logMessage("								AL_INVALID_NAME");
  //          errorAL = true;
  //      break;

  //      case AL_INVALID_ENUM:
  //          CnotiLogManager::getSingleton().getLog(soundLog)->logMessage("								AL_INVALID_ENUM");
  //          errorAL = true;
  //      break;

  //      case AL_INVALID_VALUE:
  //          CnotiLogManager::getSingleton().getLog(soundLog)->logMessage("								AL_INVALID_VALUE");
  //          errorAL = true;
  //      break;

  //      case AL_INVALID_OPERATION:
  //          CnotiLogManager::getSingleton().getLog(soundLog)->logMessage("								AL_INVALID_OPERATION");
  //          errorAL = true;
  //      break;

  //      case AL_OUT_OF_MEMORY:
  //          CnotiLogManager::getSingleton().getLog(soundLog)->logMessage("								AL_OUT_OF_MEMORY");
  //          errorAL = true;
  //      break;
  //  };
