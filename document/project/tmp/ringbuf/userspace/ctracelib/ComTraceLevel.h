#ifndef _COM_C_TRACE_LEVEL_H_
#define _COM_C_TRACE_LEVEL_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    CtlNone,
    CtlError,   /**< Serious non-recoverable fault. */
    CtlWarning, /**< Recoverable fault or condition. */
    CtlNotice,  /**< Used together with GtlWarning when, for example, a condition has recovered. */
    CtlInfo,    /**< Information. Use relatively sparsely, for example, when creating/starting/loading modules or when other important non-fault conditions occur. */
    CtlDebug,   /**< Standard debug level. Used when data, call paths etc needs to be traced. Be very restrictive when used in a live node! */
    CtlDebug2,  /**< Verbose debug level. May seriously degrade performance and cause timeouts to occur. Never to be enabled in a live node! */
    CtlDebug3,  /**< Verbose debug level. May seriously degrade performance and cause timeouts to occur. Never to be enabled in a live node! */
} ComTraceLevel;

ComTraceLevel CTL_textToTraceLevel(const char * traceLevelText);
const char * CTL_traceLevelToText(ComTraceLevel traceLevel);

#ifdef __cplusplus
}
#endif

#endif
