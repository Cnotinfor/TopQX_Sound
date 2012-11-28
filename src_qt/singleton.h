#ifndef SINGLETON_H
#define SINGLETON_H
 /*!
	\internal
	\class Singleton singleton.h <Singleton>

	Singleton is a Template you can use to make your class unique throughout the application.
 */

#include <QMutex>
#include <QMutexLocker>

template< class T >
class Singleton
{
private:
    static T* _currentInstance;
    static QMutex* _mutex;
public:
        /*!
        With this method you can get the instance of the class.
        \return Class instance.
        */
    static T* instance()
    {
        QMutexLocker lock(_mutex);
        if( Singleton<T>::_currentInstance == NULL )
        {
            Singleton<T>::_currentInstance = new T;
        }
        return Singleton<T>::_currentInstance;
    }

        /*!
        With this method you can delete the instance of the class.
        */
    static void deleteInstance()
    {
        QMutexLocker lock(_mutex);
        if( Singleton<T>::_currentInstance != NULL )
        {
            delete Singleton<T>::_currentInstance;
            Singleton<T>::_currentInstance = NULL;
        }
    }
};

template< class C >
C* Singleton<C>::_currentInstance = NULL;
template< class C >
QMutex* Singleton<C>::_mutex = new QMutex();
#endif // SINGLETON_H
