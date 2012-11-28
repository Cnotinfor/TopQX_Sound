/*!
	\class CnotiAudio::Stream
	\brief Handles ogg files.

	Emits signals with the sound name when the sample is started (soundPlaying()), 
	is paused (soundPaused()) an is stopped (soundStopped()), unless they are deactivated on playSound().

	\version 2.0
	\data 11-11-2008
	\file Stream.h
*/
#if !defined(_CNOTISOUNDSTREAM_H)
#define _CNOTISOUNDSTREAM_H

#include "SoundBase.h"
//
// OGG files
//
#ifdef _WIN32
#include "Vorbis/vorbisfile.h"
#else
#ifndef __MACOSX__
#define __MACOSX__
#endif
#include "Vorbis/vorbisfile.h"
#endif
//
// Function pointers
//
typedef int (*LPOVCLEAR)(OggVorbis_File *vf);
typedef long (*LPOVREAD)(OggVorbis_File *vf,char *buffer,int length,int bigendianp,int word,int sgned,int *bitstream);
typedef ogg_int64_t (*LPOVPCMTOTAL)(OggVorbis_File *vf,int i);
typedef vorbis_info * (*LPOVINFO)(OggVorbis_File *vf,int link);
typedef vorbis_comment * (*LPOVCOMMENT)(OggVorbis_File *vf,int link);
typedef int (*LPOVOPENCALLBACKS)(void *datasource, OggVorbis_File *vf,char *initial, long ibytes, ov_callbacks callbacks);

extern LPOVCLEAR					fn_ov_clear;
extern LPOVREAD						fn_ov_read;
extern LPOVPCMTOTAL					fn_ov_pcm_total;
extern LPOVINFO						fn_ov_info;
extern LPOVCOMMENT					fn_ov_comment;
extern LPOVOPENCALLBACKS			fn_ov_open_callbacks;

#define NUMBUFFERSOGG              (4)

namespace CnotiAudio
{
//	#define CS_REFRESH				(20)
	class Stream: public SoundBase
	{
		Q_OBJECT

	public:	
		Stream(const QString name);
		Stream(Stream &other);
		Stream(Stream &other, const QString newName);
		~Stream();

		bool load(const QString filename);
		void release();

		bool playSound( bool loop = false, bool blockSignal = false );
		bool pauseSound();
		bool stopSound();

		bool isEmpty();

		bool compareSound(SoundBase* second);
		float percentPlay();

		bool save(const QString filename);

		CnotiErrorSound getLastErrorMelody(int melody = 0);

		unsigned long getSize(){return _size;};
		ALint getFrequency(){ return _ulFrequency;};

	protected:
		void update();
		bool startStreaming();
		unsigned long DecodeOggVorbis( OggVorbis_File *psOggVorbisFile, char *pDecodeBuffer, unsigned long ulBufferSize, unsigned long ulChannels );
	
		bool                        _streamingStarted;			// Keeps if streaming is active
		QString                     _filename;					// Name of the file to stream

		ALuint                      _uiBuffer;					// Buffer to play
		ALuint                      _uiBuffers[NUMBUFFERSOGG];	// Buffer
		ALint                       _iBuffersProcessed;			// Number of process buffers
		ALint                       _iTotalBuffersProcessed;	// Total process buffers
		ALint                       _iQueuedBuffers;			// Number of buffers queued
		unsigned long               _ulChannels;				// Sound chanels
		unsigned long               _ulFormat;					// Sound format
		unsigned long               _ulBufferSize;				// Sound buffer size
		unsigned long               _ulBytesWritten;			// Sound bytes written
		ALint                       _ulFrequency;				// Sound frequency
		char*                       _pDecodeBuffer;				// ...

		FILE*                       _oggFile;					// File pointer
		ov_callbacks                _sCallbacks;				// ...
		OggVorbis_File*             _sOggVorbisFile;			// ...
		vorbis_info*                _psVorbisInfo;				// ...
	};
}

#endif  //_CNOTISOUNDSTREAM_H
