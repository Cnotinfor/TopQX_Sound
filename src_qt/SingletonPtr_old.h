#ifndef _SINGLETONPTR_H_
#define _SINGLETONPTR_H_

#include <assert.h>
#include "soundmanager_global.h"

/************************************************************************/
/* 
	Singleton template class
*/
/************************************************************************/
template <typename T> class Singleton
{
public:
	static T& getSingleton( void )
	{ 
		if(_Singleton == 0 )  // is it the first call?
		{  
			_Singleton = new T(); // create sole instance
		}
		return *_Singleton;
	}

	static T* getSingletonPtr( void )
	{ 
		if(_Singleton == 0 )  // is it the first call?
		{  
			_Singleton = new T(); // create sole instance
		}
		//assert( !_Singleton );
		return _Singleton; 
	}

	static void Destroy()
	{
		if( _Singleton )
		{
			delete _Singleton;
			_Singleton = 0;
		}
	}

protected:
	Singleton( void )
	{
		//assert( _Singleton );
		_Singleton = static_cast<T*>( this );
	}
	Singleton( const Singleton& );
	Singleton& operator= ( const Singleton& );

protected:
	// instance
	static T* _Singleton;
};

#endif	// _SINGLETO_HN_
