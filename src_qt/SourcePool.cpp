/**
	\file SourcePool.cpp
*/
#include "SourcePool.h"
// Qt
#include <QDebug>

namespace CnotiAudio
{
/*!
	Constructs an empty pool that will have \a maxSize sound sources.
*/
	SourcePool::SourcePool( int maxSize, int startSize ) :
		_maxSize (maxSize)
	{
		for (int i = 0; i < startSize; ++i)
		{
			createItem();
		}
	}

/*!
	Destroyes the source pool.
*/
	SourcePool::~SourcePool() 
	{
		int counter = 0;
		ALuint* sourceArray = new ALuint[_maxSize];	
		
		_mutex.lock();	
		//
		// Delete PoolItens
		//
		while( !_checkedOut.empty() )
		{
			sourceArray[counter++] = _checkedOut[0];
		}
		//
		// Delete sources
		//
		alDeleteSources( counter, sourceArray );

		_mutex.unlock();
	}

/*!
	Checkout Source from pool.
*/
	ALuint SourcePool::checkOut()
	{
		QMutexLocker mLocker( &_mutex );

		ALuint source= findFreeItem();
		if( source != 0 )
		{
			_checkedOut << source;
		}
		qDebug() << "[SourcePool::checkOut] out:" << _checkedOut.size() << "in:" << _queue.size();
		return source;
	}
/*!
	Checkin Source into pool      
*/
	void SourcePool::checkIn( ALuint source ) 
	{
		_mutex.lock();

		for( int i = 0; i < _checkedOut.size(); ++i )
		{
			if( _checkedOut[i] == source )
			{
				resetSourceToDefaultValues( _checkedOut[i] );
				_queue.enqueue(_checkedOut[i]);
				_checkedOut.removeAt(i);
//				qDebug() << "[SourcePool::checkIn] Source:" << source;
				break;
			}
		}

		qDebug() << "[SourcePool::checkIn] out:" << _checkedOut.size() << "in:" << _queue.size();
		_mutex.unlock();
	}

/*!
	Find Source from available sources. If none is availabre tries to create a new one.
*/
	ALuint SourcePool::findFreeItem()
	{
		// Check if queue is not empty
		if(_queue.isEmpty())
		{
			// Creates a new item
			if(!createItem())
			{
				return 0;
			}
		}
		// Return
		return _queue.dequeue();
	}

/*!
	Create a source and adds it to the queque
*/
	bool SourcePool::createItem()
	{
		ALuint uiSource = 0;
		if( _checkedOut.size() + _queue.size() >= _maxSize )
		{
			qWarning() << "[SourcePool::createItem] Number of maximum sources (" << _maxSize << ") reached." ;
			return false;
		}

		//
		// Creates a new source
		//
		bool created = false;
		while(!created)
		{
			alGetError(); // clear AL error
			alGenSources( 1, &uiSource );
			int error = alGetError();
			// Check for errors on creating the source
			if( error != AL_NO_ERROR )
			{
				if(error == AL_OUT_OF_MEMORY)
				{
					qWarning() << "[SourcePool::createItem] There is not enough memory to generate all the requested sources.";
				}
				else if (error == AL_INVALID_VALUE)
				{
					qWarning() << "[SourcePool::createItem] There are not enough non-memory resources to create all the requested sources, or the array pointer is not valid.";
				}
				else if (error == AL_INVALID_OPERATION)
				{
					qWarning() << "[SourcePool::createItem] There is no context to create sources in.";
				}
				else
				{
					qWarning() << "[SourcePool::createItem] Unknown error.";
				}
				return false;
			}
			else if (uiSource == 0)
			{
				qWarning() << "[SourcePool::createItem] Unknown error.";
				return false;
			}
			else
			{
				// Check if exists one with the same id (it shouldn't happen)
				if (!_checkedOut.contains(uiSource) && !_queue.contains(uiSource))
				{
					// Add the source to the queue
					_queue.enqueue(uiSource);
					created = true;
				}
			}
		}
		return true;
	}

/*!
	Resets the source to default values, so when is used again is doesn't 
	take anything fron the previous utilization.
*/
	void SourcePool::resetSourceToDefaultValues( ALuint uiSource )
	{
		//
		// The NULL Buffer ( 0 ) detaches buffers attached using 
		// alSourcei() or alSourceQueueBuffers().
		//
		alSourcei( uiSource, AL_BUFFER, 0 );

		//
		// Reset source values (values of a source when is created)
		//
		alSourcei( uiSource, AL_PITCH, 1 );
		alSourcei( uiSource, AL_GAIN, 1 );
		alSourcei( uiSource, AL_MIN_GAIN, 0 );
		alSourcei( uiSource, AL_MAX_GAIN, 1 );
		alSourcef( uiSource, AL_MAX_DISTANCE, 3.4028235e+038 );
		alSourcei( uiSource, AL_ROLLOFF_FACTOR, 1 );
		alSourcei( uiSource, AL_CONE_OUTER_GAIN, 0 );
		alSourcei( uiSource, AL_CONE_INNER_ANGLE, 360 );
		alSourcei( uiSource, AL_CONE_OUTER_ANGLE, 360 );
		alSourcei( uiSource, AL_REFERENCE_DISTANCE, 1 );

		alSource3i( uiSource, AL_POSITION, 0, 0, 0 );
		alSource3i( uiSource, AL_VELOCITY, 0, 0, 0 );
		alSource3i( uiSource, AL_DIRECTION, 0, 0, 0 );
	}
}
