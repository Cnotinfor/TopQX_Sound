#ifndef LOGMANAGER_GLOBAL_H
#define LOGMANAGER_GLOBAL_H

#include <qglobal.h>

#ifdef CNOTI_SHARED_LIB
# ifdef LOGMANAGER_LIB
#  define LOGMANAGER_EXPORT Q_DECL_EXPORT
# else
#  define LOGMANAGER_EXPORT Q_DECL_IMPORT
# endif
#else
# define LOGMANAGER_EXPORT
#endif

#endif // LOGMANAGER_GLOBAL_H
