#ifndef _COM_C_TRACE_HDR_OPTION_H_
#define _COM_C_TRACE_HDR_OPTION_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    logNone         = 0,    /**< Log no extra options */
    logLocation     = 1<<0, /**< Log board location */
    logTime         = 1<<1, /**< Log date & time */
    logTraceLevel   = 1<<2, /**< Log trace level of the message */
    logModuleName   = 1<<3, /**< Log module name */
    logThreadId     = 1<<4, /**< Log thread id number */
    logFileAndLine  = 1<<5, /**< Log filename and line of the message */
    logProcessName  = 1<<6  /**< Log process name */
} ComTraceHdrOption;

ComTraceHdrOption operator|(const ComTraceHdrOption a, const ComTraceHdrOption b); /**< operator for returning a bitwise OR value on two GsnTraceHdrOption variables. */
ComTraceHdrOption operator&(const ComTraceHdrOption a, const ComTraceHdrOption b); /**< operator for returning a bitwise AND value on two GsnTraceHdrOption variables. */
ComTraceHdrOption operator~(const ComTraceHdrOption a); 						   /**< operator for returning a two-complement value of a GsnTraceHdrOption variable. */

#ifdef __cplusplus
}
#endif

#endif
