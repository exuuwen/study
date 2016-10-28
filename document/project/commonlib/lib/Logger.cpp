#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Logger.h"

Logger::Logger(TraceLevel level)
	: fd_(-1),
	  mutex_(),
	  level_(level)
{
	pBuffer_ = buffer_;
}

Logger::~Logger()
{
}

void Logger::setLogLevel(TraceLevel logLevel)
{
	MutexLock lock(mutex_);
	level_ = logLevel;
}

TraceLevel Logger::getLogLevel() const
{
	MutexLock lock(mutex_);
	return level_;
}

int Logger::_write(bool flush, const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	vsprintf(pBuffer_, format, ap);
	va_end(ap);

	int len = strlen(pBuffer_);
	pBuffer_ += len;
	
	if (flush)
	{
		// flush the buffer to fd and reset
		write(fd_, buffer_, strlen(buffer_));
		pBuffer_ = buffer_;
	}

	return len;
}

int Logger::_vwrite(bool flush, const char *str, va_list ap)
{
	vsprintf(pBuffer_, str, ap);

	int len = strlen(pBuffer_);
	pBuffer_ += len;

	if (flush)
	{
		write(fd_, buffer_, strlen(buffer_));
		pBuffer_ = buffer_;
	}

	return len;
}

void Logger::log(TraceLevel level, const char *str, ...)
{
	// check the log level with pre-defined log level.
	if (level > level_)
	{
		return;
	}
		
	MutexLock lock(mutex_);
	
	// Log the type firstly
	switch (level)
	{
	case None:
		break;
		
	case Error:
		_write(false, "ERROR   ");
		break;
		
	case Warning:
		_write(false, "WARNING ");	
		break;
		
	case Notice:
		_write(false, "NOTICE  ");  
		break;
		
	case Info:   
		_write(false, "INFO    ");
		break;
		
	case Debug:  
		_write(false, "DEBUG   ");
		break;
		
	default:
		break;
	}
	
	// log the time
	time_t now;
	struct tm result;
	
	time(&now);
	localtime_r(&now, &result); 

	sprintf(strTime_, "%d-%02d-%02d %02d:%02d:%02d", 
	result.tm_year + 1900,
	result.tm_mon + 1,
	result.tm_mday,
	result.tm_hour, 
	result.tm_min, 
	result.tm_sec);

	_write(false, "[%s", strTime_);
	
	// log caller thread id
	_write(false, " T%x] ", getpid());
	
	// log the real info
	va_list ap;

	va_start(ap, str);
	_vwrite(false, str, ap);
	va_end(ap);
	_write(true, "\n");
}

void Logger::vlog(TraceLevel level, const char *str, va_list ap)
{
	// check the log level with pre-defined log level.
	if (level > level_)
	{
		return;
	}
		
	MutexLock lock(mutex_);

	// Log the type firstly
	switch (level)
	{
	case None:
		break;
		
	case Error:
		_write(false, "ERROR   ");
		break;
		
	case Warning:
		_write(false, "WARNING ");	
		break;
		
	case Notice:
		_write(false, "NOTICE  ");  
		break;
		
	case Info:   
		_write(false, "INFO    ");
		break;
		
	case Debug:  
		_write(false, "DEBUG   ");
		break;
		
	default:
		break;
	}

	// log the time
	time_t now;
	struct tm result;
	
	time(&now);
	localtime_r(&now, &result); 

	sprintf(strTime_, "%d-%02d-%02d %02d:%02d:%02d", 
	result.tm_year + 1900,
	result.tm_mon + 1,
	result.tm_mday,
	result.tm_hour, 
	result.tm_min, 
	result.tm_sec);

	_write(false, "[%s", strTime_);
	
	// log caller thread id
	_write(false, " T%x] ", pthread_self());
	
	// log the real info
	_vwrite(false, str, ap);
	_write(true, "\n");	
}


///////////////////////////////////////
FileLogger::FileLogger(const char *name, const char *logDir, TraceLevel level)
	: Logger(level), name_(0), logDir_(0)
	  
{
	bool ret = setLogDirFile(logDir, name);
	if(ret == false)
	{
		assert(0);
	}
}

FileLogger::~FileLogger()
{
	if (name_) 
	{
		free(name_);
	}
	
	if (logDir_) 
	{
		delete [] logDir_;
	}

	closeFile();
}

// set the log directory, not exists, create it.
bool FileLogger::setLogDirFile(const char *logDir, const char *name)
{
	MutexLock lock(mutex_);

	if (name_)
	{
		free(name_);
	}

	name_ = strdup(name);

	if (logDir_) 
	{
		free(logDir_);
	}

	if (logDir[strlen(logDir) - 1] == '/')
	{
		logDir_ = new char[strlen(logDir)+1];
		if (logDir_ == NULL)
		{
			return false;
		}
		strcpy(logDir_, logDir);
	}
	else
	{
		logDir_ = new char[strlen(logDir)+2];
		if (logDir_ == NULL)
		{
			return false;
		}
		strcpy(logDir_, logDir);
		strcat(logDir_, "/");
	}

	// If the logdir does not exist, create it.
	struct stat buf;
	stat(logDir_, &buf);
	if (!S_ISDIR(buf.st_mode))
	{
		if (mkdir(logDir_, S_IRWXU) != 0) // Read,write,execute by owner
		{
			printf("create log file failed\n");
			return false;
		}
	}
	else if (!(S_IWUSR & buf.st_mode))
	{
		// if not read, write execute for the user
		printf("log file not read, write and execute for user\n");
		assert(0);
	}
	
	// generate the file name.
	sprintf(fileName_, "%s%s.log", logDir_, name_);
	
	if (!openFile())
	{
		printf("failed to open log file: %s\n", fileName_);
		return false;
	}

	return true;
}


bool FileLogger::openFile()
{
	if (fd_ >= 0)
	{
		close(fd_);
	}

	if (access(fileName_, F_OK) != 0)
	{
		// not exist
		fd_ = open(fileName_, O_WRONLY | O_CREAT, S_IWUSR);

		if (fd_ >= 0)
		{
			fchmod(fd_, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
		}			
	}
	else
	{
		fd_ = open(fileName_, O_WRONLY | O_APPEND);
	}

	if (fd_ < 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

void FileLogger::closeFile()
{
	if (fd_ < 0)
	{
		return;
	}

	close(fd_);
}

const char * FileLogger::getLogFileName(void) const
{
	MutexLock lock(mutex_);
	return fileName_;
}

const char * FileLogger::getLogDir() const
{
	MutexLock lock(mutex_);
	return logDir_;
}


//////////////////////////////////
FdLogger::FdLogger(int fd, TraceLevel level)
	: Logger(level)
{
	bool ret = setLogFd(fd);

	if (ret == false)
	{
		assert(0);
	}
}

FdLogger::~FdLogger()
{
	
}

bool FdLogger::setLogFd(int fd)
{
	if (fd < 0)
	{
		printf("set wrong fd:%d\n", fd);
		return false;
	} 

	MutexLock lock(mutex_);
	fd_ = fd;

	return true;
}

///////////////////////////////////////////////
/*common fd log*/

FdLogger * pFdLogger = NULL;


int initFdLogger(const int fd, TraceLevel logLevel)
{
	if(pFdLogger != NULL)
	{
		delete pFdLogger;
	}

	pFdLogger = new FdLogger(fd, logLevel);
	if(pFdLogger == NULL)
	{
		printf("debug file logger create failed\n");
		return -1;
	}

	return 0;
} 

// Free the system logger
void freeFdLogger()
{
	if (pFdLogger != NULL)
	{
		delete pFdLogger;
		pFdLogger = NULL;
	}
}

// Set the system debug level
void setFdLogLevel(TraceLevel level)
{
	if (pFdLogger != NULL)
	{
		pFdLogger->setLogLevel(level);
	}	
}

// The api provided to the user level to log messages.
void logMsg(TraceLevel level, const char *str, ...)
{
	if (pFdLogger == NULL)
	{
		return;
	}
	
	va_list ap;
	va_start(ap, str);
	pFdLogger->vlog(level, str, ap);
	va_end(ap);	
}





