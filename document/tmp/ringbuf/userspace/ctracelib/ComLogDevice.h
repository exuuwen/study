#ifndef __LOG_DEVICE_H_
#define __LOG_DEVICE_H_

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
#include <pthread.h>

#include "ComTraceHdrOption.h"
#include "ComTraceType.h"
#include "ComTraceModule.h"

class LogDevice
{
public:
	virtual ~LogDevice();

	void logMsg(const char * traceLevel, const ComTraceModule * module, const char * fileName, 
				int lineNo, const char * msg,
                std::va_list args) 
	{
        beginLogMsg(traceLevel, module, fileName, lineNo);
        logMsgData(msg, args);
        endLogMsg();
    }

	void beginLogMsg(const char * traceLevel, const ComTraceModule * module, const char * fileName, int lineNo);

    void logMsgData(const char * fmt, std::va_list args);
	void endLogMsg();

protected:
	ComTraceHdrOption defHdrOptions_;

	LogDevice(ComTraceType type);   
    void makeHdr(const char * traceLevel, const ComTraceModule * module, const char * fileName, int lineNo);

    virtual void write(const char * data, size_t dataLen) = 0;

private:	
	const char * stripPath(const char * fileName);
	ComTraceType type_;
	pthread_mutex_t logMutex_;
	char buf_[0x4000];
	size_t bufIndex_;
};

#endif
