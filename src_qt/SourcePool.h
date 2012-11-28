/*!
 \class CnotiAudio::SourcePool
 \brief The SourcePool class manages the pool of sound sources. 
 
 The sound sources will be used to play the sounds.

 Case there are no sources available, no valid source will be returned.

 \version 2.1
 \date 10-11-2008
 \file SourcePool.h
*/
#if !defined(_SOURCEPOOL_H)
#define _SOURCEPOOL_H

//
// OpenAl
//
#if defined( __WIN32__ ) || defined( _WIN32 )
#include "openal\win32\Framework.h"
#else
#include "openal/MacOSX/MyOpenALSupport.h"
#endif
//
// Qt
//
#include <QMutex>
#include <QList>
#include <QQueue>

namespace CnotiAudio
{
	class SourcePool
	{
	public:
		SourcePool( int maxSize = 8, int startSize = 4 );
		~SourcePool();

		ALuint checkOut();
		void checkIn( ALuint source );

	private:
		int               _maxSize;		// Pool maximum size
		ALuint            _lastSource;	// Used because in mac, in some cases, it was not working correctly when the last was used
		QList<ALuint>     _checkedOut;	// Pool of sources being used
		QQueue<ALuint>    _queue;		// Available sources
		QMutex            _mutex;
		// Methods
		bool createItem();
		ALuint findFreeItem();
		void resetSourceToDefaultValues( ALuint source );
	};
}

#endif //_SOURCEPOOL_H
