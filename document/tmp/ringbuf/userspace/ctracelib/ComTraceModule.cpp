#include "ComTraceModule.h"

#include <cassert>
#include <cstring>

ComTraceModule::ComTraceModule(const char * moduleName, ComTraceType type, ComTraceLevel level, const char * processName)
{
	assert(moduleName);
    moduleName_ = new char[strlen(moduleName)+1];
    strcpy(moduleName_, moduleName);

    lvl_ = level;
	type_ = type;
    hdrOptions_ = logNone;

    if (processName == 0) 
	{
		//TODO
        processName = "test";
    }
    processName_ = new char[strlen(processName)+1];
    strcpy(processName_, processName);
}

void ComTraceModule::setHeaderOptions(ComTraceHdrOption hdrOptions)
{
	if (type_ == cttRsyslog)
	{
		hdrOptions = hdrOptions & (~logTime);
		hdrOptions = hdrOptions & (~logProcessName);
	}
	else if (type_ == cttRingbuflog)
	{
		hdrOptions = hdrOptions & (~logTime);
		hdrOptions = hdrOptions & (~logProcessName);
	}

	hdrOptions_ = hdrOptions;
}

