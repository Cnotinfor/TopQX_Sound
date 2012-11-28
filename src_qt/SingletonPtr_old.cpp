#include "SingletonPtr.h"

template<typename T> T* Singleton<T>::_Singleton = 0;
