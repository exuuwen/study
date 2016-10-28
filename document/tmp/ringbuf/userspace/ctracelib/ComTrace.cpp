#include "ComTrace.h"
#include "ComTraceType.h"
#include "FileDevice.h"
#include "RsyslogDevice.h"
#include "RingbufDevice.h"

#include <cassert>

ComTrace::ComTrace(const char* moduleName, ComCTrace* ct,  ComTraceType type, ComTraceLevel traceLevel)
	: comCTrace_(ct)
{
	module_ = new ComTraceModule(moduleName, type, traceLevel);

	if (type == cttRingbuflog)
	{
		dev_ = new RingbufDevice(module_->getProcessName());
	}
	else if (type == cttRsyslog)
		dev_ = new RsyslogDevice();
	else
		dev_ = new FileDevice(module_->getProcessName());
}

ComTrace::~ComTrace()
{
	printf("in %s\n", __func__);
	delete module_;
	delete dev_;
}

void ComTrace::setLevel(ComTraceLevel lvl)
{
	//pthread_mutex_lock(&loggersMutex_);

	module_->setTraceLevel(lvl);
	if (comCTrace_ != 0) 
	{
		comCTrace_->traceLevel_ = lvl;
	}

	//pthread_mutex_unlock(&loggersMutex_);
}

void ComTrace::logMsg(ComTraceLevel lvl, const char* fileName, int lineNo, const char* fmt, va_list args)
{
	//pthread_mutex_lock(&loggersMutex_);

	va_list argsCopy;
    va_copy(argsCopy, args);
	dev_->logMsg(CTL_traceLevelToText(lvl), module_, fileName, lineNo, fmt, argsCopy);

	//pthread_mutex_unlock(&loggersMutex_);
}

