/*!
	\class CnotiAudio::Sample
	\brief Handles wave files.

	Emits signals with the sound name when the sample is started (soundPlaying()), 
	is paused (soundPaused()) an is stopped (soundStopped()), unless they are deactivated on playSound().

	\version 2.0
	\data 11-11-2008
	\file Sample.h
*/
#if !defined(_CNOTISOUNDSAMPLE_H)
#define _CNOTISOUNDSAMPLE_H

#include "SoundBase.h"

namespace CnotiAudio
{
//	#define CS_REFRESH				(20)
	class Sample: public SoundBase
	{
		Q_OBJECT

	public:	
		Sample( const QString name );
		Sample( Sample &other );
		Sample( Sample &other, const QString newName );
		~Sample();

		bool load( const QString filename );
		void release();

		bool playSound( bool loop = false, bool blockSignal = false );
		bool pauseSound();
		bool stopSound();

		bool isEmpty();

		bool compareSound( SoundBase* second );
		float percentPlay();

		bool save( const QString filename );

		ALuint getBuffer();

	protected:
		virtual void update();

		ALuint _buffer;
	};
}

#endif  //_CNOTISOUNDSAMPLE_H
