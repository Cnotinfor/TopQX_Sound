#ifndef CAPTURETHREAD_H
#define CAPTURETHREAD_H

#include <QThread>
#include <QMutex>
#ifdef _WIN32
// OpenAL Framework
#include "openal\win32\Framework.h"
#else
#include "openal/MacOSX/MyOpenALSupport.h"
#include "sndfile.h"
#endif


class QString;
class QStringList;

namespace CnotiAudio
{
#if defined( __WIN32__ ) || defined( _WIN32 )
	#pragma pack (push,1)
	typedef struct
	{
		char			szRIFF[4];
		long			lRIFFSize;
		char			szWave[4];
		char			szFmt[4];
		long			lFmtSize;
		WAVEFORMATEX	wfex;
		char			szData[4];
		long			lDataSize;
	} WAVEHEADER;
	#pragma pack (pop)
#endif

	class CaptureThread : public QThread
	{
		Q_OBJECT

	public:
		CaptureThread(QObject *parent = 0);
		~CaptureThread();

		QStringList deviceList();
		const QString getDevice();
		bool isCapturing();
		short* getBigBufferData( int index );
		QByteArray* getBufferListData( int index );

	public slots:
		void startCapture( const QString &filename );
		void stopCapture();

		void applyNoiseReduction( float* outBuffer, int sampleSize );
		void changeDevice( const QString &deviceName );

	signals:
		void signalCaptureEnded();
		void captureVolume( int );
		void signalSampleCaptured();

	protected:
		void run();

	private:
		static const int BUFFERSIZE	= 4410;
		ALCdevice*		_pDevice;
		ALCcontext*		_pContext;
		ALCdevice*		_pCaptureDevice;
		const ALCchar*	_szDefaultCaptureDevice;
		ALint			_iSamplesAvailable;
		ALchar			_buffer[BUFFERSIZE];
		QList<QByteArray*>  _bufferList;

#if defined( __WIN32__ ) || defined( _WIN32 )
		FILE*			_pFile;
		WAVEHEADER		_sWaveHeader;
#else
		SNDFILE*        _pFile;
		SF_INFO	        _sfInfo;
		int				_nBlockAlign;
#endif
		ALint			_iDataSize;
		ALint			_iSize;

		QMutex			_captureMutex;
		bool			_capturing;
		bool			_noiseReductionActive;  // Flag to apply or not the noise reduction filter to recorded buffer

		QString			_cDeviceName;	// Holds the name of the current device

		QString         _logFile;
		// Functions
		void initializeSound();
		void writeAudioBufferToFile( int bufferWriteSize );
	};

}
#endif // CAPTURETHREAD_H
