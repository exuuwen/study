#ifndef _COM_TRACE_H_
#define _COM_TRACE_H_

#include "ComCTrace.h"
#include "ComTraceType.h"
#include "ComLogDevice.h"
#include "ComTraceHdrOption.h"
#include "ComTraceModule.h"


class ComTrace
{
public:
    ComTrace(const char * moduleName, ComCTrace * ct,  ComTraceType type, ComTraceLevel traceLevel);
    /**
     * Copy constructor
     * 
     * @param copy Reference to the GsnTrace object to copy
     */
    //ComTrace(const ComTrace & rhs);

	~ComTrace();

	void setLevel(ComTraceLevel lvl);
	void setHeaderOptions(ComTraceHdrOption hdrOptions) { module_->setHeaderOptions(hdrOptions); }

	//void logMsg(ComTraceLevel lvl, const char * fileName, int lineNo, const char * msg);
    void logMsg(ComTraceLevel lvl, const char * fileName, int lineNo, const char * fmt, va_list args);

private:
	ComTraceModule *module_;
	ComCTrace *comCTrace_; /**< Pointer reference to ComCTrace object. This member is null if *this was not created through a CT_create() call. */
	LogDevice *dev_;
};


#endif

