#ifndef __TRACE_MODULE_H_
#define __TRACE_MODULE_H_

#include "ComTraceHdrOption.h"
#include "ComTraceLevel.h"
#include "ComTraceType.h"

#include <cstdio>

class ComTraceModule
{
public:
	ComTraceModule(const char * moduleName, ComTraceType type, ComTraceLevel traceLevel = CtlNotice, const char * processName = 0);
	~ComTraceModule() 
	{ 
		printf("in %s\n", __func__);
		delete[] processName_; 
		delete[] moduleName_; 
	}

	 /** @return Name of the module */
    const char * getName() const { return moduleName_; }
    /** @return Name of the process the module belongs to. */
    const char * getProcessName() const { return processName_; }
    /** @return Trace level of the module */
    ComTraceLevel getTraceLevel() const { return lvl_; }
    /** @param lvl Trace level to set */
    void setTraceLevel(ComTraceLevel lvl) { lvl_ = lvl; }
    
    /** @return Enabled header options in the module */
    ComTraceHdrOption getHeaderOptions() const { return hdrOptions_; };

    /** 
     * Enable a diffrent subset of module header options.
     * Please note that Module header options will be added to 
     * the LogDevice header options when the header is generated
     *
     * @param hdrOptions Header options to enable when generating headerfiles for this module 
     */
    void setHeaderOptions(ComTraceHdrOption hdrOptions);

private:
    char * moduleName_;
    char * processName_;
    ComTraceLevel lvl_;
	ComTraceType type_;
    ComTraceHdrOption hdrOptions_;
};
#endif
