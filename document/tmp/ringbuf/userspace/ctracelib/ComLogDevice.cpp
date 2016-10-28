#include "ComLogDevice.h"

LogDevice::LogDevice(ComTraceType type)
	: type_(type)
{
	std::memset(&logMutex_, 0, sizeof(pthread_mutex_t));
    pthread_mutex_init(&logMutex_, NULL);
}


LogDevice::~LogDevice()
{
    pthread_mutex_destroy(&logMutex_);
	printf("in %s\n", __func__);
}

void LogDevice::beginLogMsg(const char * traceLevel, const ComTraceModule * module, const char * fileName, int lineNo)
{
	pthread_mutex_lock(&logMutex_);
	bzero(buf_, sizeof(buf_));
	makeHdr(traceLevel, module, fileName, lineNo);
}

void LogDevice::endLogMsg()
{
	write(buf_, bufIndex_);
	pthread_mutex_unlock(&logMutex_);
}

void LogDevice::logMsgData(const char * fmt, std::va_list args)
{
	bufIndex_ += ::vsnprintf(buf_ + bufIndex_, sizeof(buf_) - bufIndex_, fmt, args);
	if (bufIndex_ > sizeof(buf_)) 
	{
		bufIndex_ = sizeof(buf_);
	}
}

void LogDevice::makeHdr(const char * traceLevel, const ComTraceModule * module, const char * fileName, int lineNo)
{
	bufIndex_ = 0;

	ComTraceHdrOption hdrOptions = defHdrOptions_ | module->getHeaderOptions();	

	if (hdrOptions & logTime) 
	{
		time_t nowTime = time(0);
		struct tm nowTm;
		gmtime_r(&nowTime, &nowTm);    	
		bufIndex_ += strftime(buf_ + bufIndex_, sizeof(buf_) - bufIndex_, "%Y-%m-%d %H:%M:%S(GMT) ", &nowTm);
	}

	
	if (hdrOptions & logProcessName) 
	{
		std::strcpy(buf_ + bufIndex_, module->getProcessName());
		bufIndex_ += std::strlen(buf_ + bufIndex_);
		buf_[bufIndex_++] = ':';
		buf_[bufIndex_++] = ' ';
	}
	
	if (hdrOptions & logModuleName) 
	{
		std::strcpy(buf_ + bufIndex_, module->getName());
		bufIndex_ += std::strlen(buf_ + bufIndex_);
	}

	if ((hdrOptions & logTraceLevel) && traceLevel != 0) 
	{
		bufIndex_ += std::sprintf(buf_ + bufIndex_, " %-7s", traceLevel);
		buf_[bufIndex_++] = ':';
		buf_[bufIndex_++] = ' ';
	}

	if (hdrOptions & logThreadId) 
	{
		bufIndex_ += std::sprintf(buf_ + bufIndex_, " %lu: ", (unsigned long)pthread_self());
	}

	if ((hdrOptions & logFileAndLine) && fileName != 0)
	{
		std::strcpy(buf_ + bufIndex_, stripPath(fileName));
		bufIndex_ += std::strlen(buf_ + bufIndex_);
		bufIndex_ += std::sprintf(buf_ + bufIndex_, " %d: ", lineNo);
	}
}

const char * LogDevice::stripPath(const char * fileName)
{
    const char * strippedName = std::strrchr(fileName, '/');
    if (strippedName++ != 0)
    {
        return strippedName;
    }

    return fileName;
}



