//#include <assert.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <ctime>
//#include <cstring>
#include <cstdarg>
#include <cmath>

// STL containers
#include <iomanip>
//#include <vector>
#include <map>
#include <string>
#include <set>
#include <list>
#include <deque>
#include <queue>
#include <bitset>
#include <algorithm>

#include "logmanager.h"

/*!
	\class CnotiLogManager

	\ingroup Misc
*/



//namespace Logger 
//{

/************************************************************************/
/* 
LOG
*/
/************************************************************************/

//-----------------------------------------------------------------------
Log::Log( const QString& name, bool debuggerOuput, bool suppressFile ) : 
mLogLevel(LL_NORMAL), mDebugOut(debuggerOuput),
mSuppressFile(suppressFile), mLogName(name)
{
	if (!mSuppressFile)
	{
		mfpLog.open(name.toStdString().c_str());
	}

	if( !mSuppressFile )
	{
		struct tm *pTime;
		time_t ctTime; time(&ctTime);
		pTime = localtime( &ctTime );
		mfpLog << "Cnotinfor Log File @ " << std::setw(2) << std::setfill('0') << pTime->tm_mday
			<< "-" << std::setw(2) << std::setfill('0') << pTime->tm_mon+1
			<< "-" << std::setw(2) << std::setfill('0') << pTime->tm_year+1900
			<< std::endl << std::endl;

		// Flush stcmdream to ensure it is written (incase of a crash, we need log to be up to date)
		mfpLog.flush();
	}

}
//-----------------------------------------------------------------------
Log::~Log()
{
	if (!mSuppressFile)
	{
		mfpLog.close();
	}
}
//-----------------------------------------------------------------------
void Log::logMessage( const QString& message, LogMessageLevel lml, bool maskDebug )
{
	if ((mLogLevel + lml) >= OGRE_LOG_THRESHOLD)
	{
		for( mtLogListener::iterator i = mListeners.begin(); i != mListeners.end(); ++i )
			(*i)->messageLogged( message, lml, maskDebug, mLogName );

		if (mDebugOut && !maskDebug) std::cerr << message.toStdString() << std::endl;

		// Write time into log
		if( !mSuppressFile )
		{
			struct tm *pTime;
			time_t ctTime; time(&ctTime);
			pTime = localtime( &ctTime );
			mfpLog << std::setw(2) << std::setfill('0') << pTime->tm_hour
				<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_min
				<< ":" << std::setw(2) << std::setfill('0') << pTime->tm_sec 
				<< ": " << message.toStdString() << std::endl;

			// Flush stcmdream to ensure it is written (incase of a crash, we need log to be up to date)
			mfpLog.flush();
		}
	}
}

//-----------------------------------------------------------------------
void Log::setDebugOutputEnabled( bool debugOutput )
{
	mDebugOut = debugOutput;
}

//-----------------------------------------------------------------------
void Log::setLogDetail( LoggingLevel ll )
{
	mLogLevel = ll;
}

//-----------------------------------------------------------------------
void Log::addListener( LogListener* listener )
{
	mListeners.push_back(listener);
}

//-----------------------------------------------------------------------
void Log::removeListener( LogListener* listener )
{
	mListeners.erase( std::find(mListeners.begin(), mListeners.end(), listener) );
}




/************************************************************************/
/* 
LOGMANAGER
*/
/************************************************************************/

//-----------------------------------------------------------------------
CnotiLogManager CnotiLogManager::ms_Singleton;


QString CnotiLogManager::number(const float f)
{
	return QString::number( f );
}

QString CnotiLogManager::number(const int n, const int radix)
{
	return QString::number( n, radix );
}	

QString CnotiLogManager::boolean(const bool value)
{
	if(value)
		return "true";
	else
		return "false";
}
CnotiLogManager* CnotiLogManager::getSingletonPtr(void)
{
	return &ms_Singleton;
}
CnotiLogManager& CnotiLogManager::getSingleton(void)
{  
	return ( ms_Singleton );  
}

//-----------------------------------------------------------------------
CnotiLogManager::CnotiLogManager()
{
	mDefaultLog = NULL;
}

//-----------------------------------------------------------------------
CnotiLogManager::~CnotiLogManager()
{
	// Destroy all logs
	LogList::iterator i;
	for (i = mLogs.begin(); i != mLogs.end(); ++i)
	{
		delete i->second;
	}
}

//-----------------------------------------------------------------------
Log* CnotiLogManager::createLog( const QString& filename, bool defaultLog, bool debuggerOutput, bool suppressFileOutput, const QString& name)
{
	Log* newLog = new Log( filename, debuggerOutput, suppressFileOutput );

	if( !mDefaultLog || defaultLog )
	{
		mDefaultLog = newLog;
	}
	QString _name = name;
	if(_name == "")
		_name = filename;
	mLogs.insert( LogList::value_type( _name, newLog ) );

	return newLog;
}

//-----------------------------------------------------------------------
Log* CnotiLogManager::getDefaultLog()
{
	return mDefaultLog;
}

//-----------------------------------------------------------------------
Log* CnotiLogManager::setDefaultLog(Log* newLog)
{
	Log* oldLog = mDefaultLog;
	mDefaultLog = newLog;
	return oldLog;
}

//-----------------------------------------------------------------------
Log* CnotiLogManager::getLog( const QString& name)
{
	LogList::iterator i = mLogs.find(name);
	if (i != mLogs.end())
		return i->second;
	//else
	//	OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Log not found. ", "CnotiLogManager::getLog");

	return NULL;
}

//-----------------------------------------------------------------------
void CnotiLogManager::destroyLog(const QString& name)
{
	LogList::iterator i = mLogs.find(name);
	if (i != mLogs.end())
	{
		if (mDefaultLog == i->second)
		{
			mDefaultLog = 0;
		}
		delete i->second;
		mLogs.erase(i);
	}

	// Set another default log if this one removed
	if (!mDefaultLog && !mLogs.empty())
	{
		mDefaultLog = mLogs.begin()->second;
	}
}
//-----------------------------------------------------------------------
void CnotiLogManager::destroyLog(Log* log)
{
	destroyLog(log->getName());
}

//-----------------------------------------------------------------------
void CnotiLogManager::logMessage( const QString& message, LogMessageLevel lml, bool maskDebug)
{
	if (mDefaultLog)
	{
		mDefaultLog->logMessage(message, lml, maskDebug);
	}
}
//-----------------------------------------------------------------------
void CnotiLogManager::setLogDetail(LoggingLevel ll)
{
	if (mDefaultLog)
	{
		mDefaultLog->setLogDetail(ll);
	}
}

//} // namespace
