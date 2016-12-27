#include "ComTraceLevel.h"

#include <cstdio>
#include <cstring>

static const char comTraceLevelTexts[][8] = { "", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG", "DEBUG2", "DEBUG3" };

ComTraceLevel CTL_textToTraceLevel(const char * traceLevelText)
{
    for (size_t i = 0; i < sizeof(comTraceLevelTexts) / sizeof(comTraceLevelTexts[0]); i++)
    {
        if (std::strcmp(traceLevelText, comTraceLevelTexts[i]) == 0)
        {
            return (ComTraceLevel)i;
        }
    }
    
    (void)std::fprintf(stderr, "WARNING: Trace level not found: %s\n", traceLevelText);
    
    return CtlNotice;
}



const char* CTL_traceLevelToText(ComTraceLevel traceLevel)
{
    if ((size_t)traceLevel >= sizeof(comTraceLevelTexts) / sizeof(comTraceLevelTexts[0]))
    {
        (void)std::fprintf(stderr, "WARNING: Invalid trace level: %d\n", traceLevel);
        return "Undefined";
    }
        
    return comTraceLevelTexts[traceLevel];
}


