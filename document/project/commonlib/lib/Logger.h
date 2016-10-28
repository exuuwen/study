#ifndef _LIB_LOGGER_H_
#define _LIB_LOGGER_H_

#include "MutexLock.h"
#include "Base.h"

typedef enum
{
	None,    
	Error,   /**< Serious non-recoverable fault. */
	Warning, /**< Recoverable fault or condition. */
	Notice,  /**< Used together with GtlWarning when, for example, a condition has recovered. */
	Info,    /**< Information. Use relatively sparsely, for example, when creating/starting/loading modules or when other important non-fault conditions occur. */
	Debug   /**< Standard debug level. Used when data, call paths etc needs to be traced. */
} TraceLevel;

class Logger : Noncopyable
{
public:
	// This constructor is for stdout, stderr, or other opened file like pipe, socket, etc.
	//Logger(int fd, TraceLevel level = None);

	// This constructor is for file logging.
	
	virtual ~Logger();

	void setLogLevel(TraceLevel loglevel);
	TraceLevel getLogLevel() const;	
	
	void log(TraceLevel level, const char *str, ...);
	void vlog(TraceLevel level, const char *str, va_list ap);
protected:
	Logger(TraceLevel level);
	int  fd_;              // socket, pipe fd, or file fd depends.
	mutable Mutex mutex_;
private:
	int _write(bool flush, const char *str, ...);
	int _vwrite(bool flush, const char *str, va_list ap);

	TraceLevel level_; // current log level
	char buffer_[5136];    // log string buffer per one output
	char *pBuffer_;
	char strTime_[30];
	
};


class FileLogger : public Logger
{
public:
	FileLogger(const char *name, const char *logdir, TraceLevel level = None);
	virtual ~FileLogger();

	const char *getLogDir() const;	
	const char *getLogFileName(void) const;
	bool setLogDirFile(const char *logdir, const char *name);
private:
	bool openFile();  // open the log file if file log mode
	void closeFile(); // close log file if file log mode
	
	char fileName_[256];   // full path log name
	char * name_;          // log name
	char * logDir_;        // log path
};


class FdLogger : public Logger
{
public:
	FdLogger(int fd, TraceLevel level = None);
	virtual ~FdLogger();

	bool setLogFd(int fd);
};

int initFdLogger(int fd, TraceLevel loglevel);
void freeFdLogger();
void setFdLogLevel(TraceLevel level);
void logMsg(TraceLevel level, const char *str, ...);

#define logErrMsg(fmt, args...) logMsg(Error, fmt, ##args)
#define logWarnMsg(fmt, ...)   logMsg(Warning, fmt, ## __VA_ARGS__)
#define logNoticeMsg(fmt, ...) logMsg(Notice, fmt, ## __VA_ARGS__)
#define logInfoMsg(fmt, ...)   logMsg(Info, fmt, ## __VA_ARGS__)
#define logDebugMsg(fmt, ...)  logMsg(Debug, fmt, ## __VA_ARGS__)

#endif


