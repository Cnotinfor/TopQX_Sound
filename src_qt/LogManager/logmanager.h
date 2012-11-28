#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include "logmanager_global.h"

#include <iostream>
#include <fstream>
//#include <vector>
//#include <string>
#include <map>

#include <QString>
#include <QList>
//namespace Logger
//{

/************************************************************************/
/* 
LOG
*/
/************************************************************************/

// LogMessageLevel + LoggingLevel > OGRE_LOG_THRESHOLD = message logged
#define OGRE_LOG_THRESHOLD 4

/** The level of detail to which the log will go into.
*/
enum LoggingLevel
{
	LL_LOW = 1,
	LL_NORMAL = 2,
	LL_BOREME = 3
};

/** The importance of a logged message.
*/
enum LogMessageLevel
{
	LML_TRIVIAL = 1,
	LML_NORMAL = 2,
	LML_CRITICAL = 3
};

/** @remarks Pure Abstract class, derive this class and register to the Log to listen to log messages */
class LOGMANAGER_EXPORT LogListener
{
public:
	virtual ~LogListener() {};

	/**
	@remarks
	This is called whenever the log receives a message and is about to write it out
	@param message
	The message to be logged
	@param lml
	The message level the log is using
	@param maskDebug
	If we are printing to the console or not
	@param logName
	the name of this log (so you can have several listeners for different logs, and identify them)
	*/
	virtual void messageLogged( const QString& message, LogMessageLevel lml, bool maskDebug, const QString &logName ) = 0;
};

/**
@remarks
Log class for writing debug/log data to files.
@note
<br>Should not be used directly, but trough the CnotiLogManager class.
*/
class LOGMANAGER_EXPORT Log
{
protected:
	std::ofstream  mfpLog;
	LoggingLevel   mLogLevel;
	bool           mDebugOut;
	bool           mSuppressFile;
	QString        mLogName;

	typedef QList<LogListener*> mtLogListener;
	mtLogListener	mListeners;

public:
	/**
	@remarks
	Usual constructor - called by CnotiLogManager.
	*/
	Log( const QString& name, bool debugOutput = true, bool suppressFileOutput = false);

	/**
	@remarks
	Default destructor.
	*/
	~Log();

	/// Return the name of the log
	const QString& getName() const	{ return mLogName; }
	/// Get whether debug output is enabled for this log
	bool isDebugOutputEnabled() const	{ return mDebugOut; }
	/// Get whether file output is suppressed for this log
	bool isFileOutputSuppressed() const	{ return mSuppressFile; }

	/** Log a message to the debugger and to log file (the default is "<code>OGRE.log</code>"),
	*/
	void logMessage( const QString& message, LogMessageLevel lml = LML_NORMAL, bool maskDebug = false );

	/**
	@remarks
	Enable or disable outputting log messages to the debugger.
	*/
	void setDebugOutputEnabled(bool debugOutput);
	/**
	@remarks
	Sets the level of the log detail.
	*/
	void setLogDetail(LoggingLevel ll);
	/** Gets the level of the log detail.
	*/
	LoggingLevel getLogDetail() const { return mLogLevel; }
	/**
	@remarks
	Register a listener to this log
	@param
	A valid listener derived class
	*/
	void addListener(LogListener* listener);

	/**
	@remarks
	Unregister a listener from this log
	@param
	A valid listener derived class
	*/
	void removeListener(LogListener* listener);
};



/************************************************************************/
/* 
LOGMANAGER
*/
/************************************************************************/

/** The log manager handles the creation and retrieval of logs for the
application.
@remarks
This class will create new log files and will retrieve instances
of existing ones. Other classes wishing to log output can either
create a fresh log or retrieve an existing one to output to.
One log is the default log, and is the one written to when the
logging methods of this class are called.
@par
By default, Root will instantiate a CnotiLogManager (which becomes the 
Singleton instance) on construction, and will create a default log
based on the Root construction parameters. If you want more control,
for example redirecting log output right from the start or suppressing
debug output, you need to create a CnotiLogManager yourself before creating
a Root instance, then create a default log. Root will detect that 
you've created one yourself and won't create one of its own, thus
using all your logging preferences from the first instance.
*/
class LOGMANAGER_EXPORT CnotiLogManager //: public Singleton<CnotiLogManager>
{
protected:
	typedef std::map<QString, Log*, std::less<QString> >	LogList;

	/// A list of all the logs the manager can access
	LogList mLogs;

	/// The default log to which output is done
	Log* mDefaultLog;

private:
	static CnotiLogManager ms_Singleton;

public:
	CnotiLogManager();
	~CnotiLogManager();

	/** Creates a new log with the given name.
	@param
	name The name to give the log e.g. 'Ogre.log'
	@param
	defaultLog If true, this is the default log output will be
	sent to if the generic logging methods on this class are
	used. The first log created is always the default log unless
	this parameter is set.
	@param
	debuggerOutput If true, output to this log will also be
	routed to the debugger's output window.
	@param
	suppressFileOutput If true, this is a logical rather than a physical
	log and no file output will be written. If you do this you should
	register a LogListener so log output is not lost.
	*/
	Log* createLog( const QString& filename, bool defaultLog = false, bool debuggerOutput = true, 
		bool suppressFileOutput = false, const QString& name = "");


	/** Retrieves a log managed by this class.
	*/
	Log* getLog( const QString& name);


	/** Returns a pointer to the default log.
	*/
	Log* getDefaultLog();


	/** Closes and removes a named log. */
	void destroyLog(const QString& name);


	/** Closes and removes a log. */
	void destroyLog(Log* log);


	/** Sets the passed in log as the default log.
	@returns The previous default log.
	*/
	Log* setDefaultLog(Log* newLog);


	/** Log a message to the default log.
	*/
	void logMessage( const QString& message, LogMessageLevel lml = LML_NORMAL, bool maskDebug = false);


	/** Log a message to the default log (signature for backward compatibility).
	*/
	void logMessage( LogMessageLevel lml, const QString& message,  bool maskDebug = false) 
	{ 
		logMessage(message, lml, maskDebug); 
	}


	/** Sets the level of detail of the default log.
	*/
	void setLogDetail(LoggingLevel ll);

	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/

	static CnotiLogManager& getSingleton(void);
	/** Override standard Singleton retrieval.
	@remarks
	Why do we do this? Well, it's because the Singleton
	implementation is in a .h file, which means it gets compiled
	into anybody who includes it. This is needed for the
	Singleton template to work, but we actually only want it
	compiled into the implementation of the class based on the
	Singleton, not all of them. If we don't change this, we get
	link errors when trying to use the Singleton-based class from
	an outside dll.
	@par
	This method just delegates to the template version anyway,
	but the implementation stays in this single compilation unit,
	preventing link errors.
	*/
	static CnotiLogManager* getSingletonPtr(void);
	static QString number(const int n,  const int radix=10);
	static QString number(const float f);
	static QString boolean(const bool value);
};
//}

#endif // LOGMANAGER_H
