#ifndef _COM_C_TRACE_H_
#define _COM_C_TRACE_H_

#include <limits.h>
#include <stdarg.h>
#include <time.h>

#include "ComTraceLevel.h"
#include "ComTraceHdrOption.h"
#include "ComTraceType.h"

#ifdef __cplusplus
extern "C" {
#endif

#define __HERE__ __FILE__, __LINE__

typedef struct 
{
    ComTraceLevel traceLevel_; /**< Current trace level of the GsnCTrace object */ 
    void * comTrace_; /**< @internal Pointer to GsnTrace object */
} ComCTrace;

void CT_printf(ComCTrace * self, ComTraceLevel lvl, const char * filename, int lineno, const char * fmt, ...);
void CT_vprintf(ComCTrace * self, ComTraceLevel lvl, const char * filename, int lineno, const char * fmt, va_list va);

void CT_setLevel(ComCTrace * self, ComTraceLevel traceLevel);

ComCTrace* CT_create(const char * moduleName, ComTraceType traceType, ComTraceLevel traceLevel);
ComCTrace* CT_createEx(const char * moduleName, ComTraceType traceType, ComTraceLevel traceLevel, ComTraceHdrOption options);


void CT_setHeaderOptions(ComCTrace * self, const ComTraceHdrOption option);

void CT_destroy(ComCTrace * self);


#define CT_isEnabled(self, lvl) ((self)->traceLevel_ >= lvl ? 1 : 0)

#define CT_condPrintf(self, lvl, fmt...) \
    do { \
        if (__builtin_expect((self)->traceLevel_ >= (lvl), 0)) \
        { \
            CT_printf(self, lvl, __HERE__, fmt); \
        } \
    } while (0)

/**
 * Conditional error message macro
 *
 * \param self  Pointer to a ComCTrace object
 * \param fmt   Format string to print
 * \param ...   Variable number of arguments to the format string
 *
 * A message is only printed if the current trace level of the GsnCTrace object is
 * higher than or equal to GtlError for this macro (CtlWarning, CtlNotice, CtlInfo, 
 * CtlDebug, CtlDebug2 or CtlDebug3 for the other macros).
 * Otherwise nothing is done.
 *
 * @note CT_error (and CT_warning, CT_info etc) : It writes directly to
 *       ringbuf and cannot log to files.
 *        
 * Example:
 * \code
 * CT_error(self, "Hello %s\n", "World");
 * \endcode
 */

#define CT_error(self, fmt...) CT_condPrintf(self, CtlError, fmt)
/**
 * Conditional warning message macro
 *
 * See CT_error() for info on parameters and usage.
 */
#define CT_warning(self, fmt...) CT_condPrintf(self, CtlWarning, fmt)
/**
 * Conditional notice message macro
 *
 * See CT_error() for info on parameters and usage.
 */
#define CT_notice(self, fmt...) CT_condPrintf(self, CtlNotice, fmt)
/**
 * Conditional info message macro
 *
 * See CT_error() for info on parameters and usage.
 */
#define CT_info(self, fmt...) CT_condPrintf(self, CtlInfo, fmt)
/**
 * Conditional debug message macro
 *
 * See CT_error() for info on parameters and usage.
 */
#define CT_debug(self, fmt...) CT_condPrintf(self, CtlDebug, fmt)
/**
 * Conditional verbose debug message macro
 *
 * See CT_error() for info on parameters and usage.
 */
#define CT_debug2(self, fmt...) CT_condPrintf(self, CtlDebug2, fmt)
/**
 * Conditional verbose debug message macro
 *
 * See CT_error() for info on parameters and usage.
 */
#define CT_debug3(self, fmt...) CT_condPrintf(self, CtlDebug3, fmt)

#ifdef __cplusplus
}
#endif


#endif
