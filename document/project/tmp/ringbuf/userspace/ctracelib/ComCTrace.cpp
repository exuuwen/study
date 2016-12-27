#include <cassert>
#include <cstdlib>

#include "ComCTrace.h"
#include "ComTrace.h"

ComCTrace* CT_create(const char * moduleName, ComTraceType traceType, ComTraceLevel traceLevel)
{
	ComCTrace * self;

	self = (ComCTrace *)malloc(sizeof(ComCTrace));
	assert(self);
    self->traceLevel_ = traceLevel;

	self->comTrace_ = (void *)new ComTrace(moduleName, self, traceType, traceLevel);

    return self;
}

ComCTrace * CT_createEx(const char * moduleName, ComTraceType traceType, ComTraceLevel traceLevel, ComTraceHdrOption options)
{
	ComCTrace * self = CT_create(moduleName, traceType, traceLevel);
	CT_setHeaderOptions(self, options);
	
	return self;
}

void CT_destroy(ComCTrace * self)
{
	if (self == 0)
	{
		return;
	}
	
	delete (ComTrace *)self->comTrace_;
	free(self);	
}

void CT_setHeaderOptions(ComCTrace * self, const ComTraceHdrOption options)
{
    ((ComTrace *)self->comTrace_)->setHeaderOptions(options);
}

void CT_printf(ComCTrace * self, ComTraceLevel lvl, const char * fileName, int lineNo, const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    CT_vprintf(self, lvl, fileName, lineNo, fmt, args);
    va_end(args);
}

void CT_vprintf(ComCTrace* self, ComTraceLevel lvl, const char * fileName, int lineNo, const char * fmt, va_list args)
{
    ((ComTrace *)self->comTrace_)->logMsg(lvl, fileName, lineNo, fmt, args);
}

void CT_setLevel(ComCTrace* self, ComTraceLevel traceLevel)
{
    ((ComTrace *)self->comTrace_)->setLevel(traceLevel);
}





