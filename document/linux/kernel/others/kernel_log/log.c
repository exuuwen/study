#include "log.h"

#include <linux/time.h>

KernelTraceLevel kernel_current_loglevel = KernelInfo;

// Kernel logger wrapper
// Implemented a debug level based mechnism
int uprintk(KernelTraceLevel level, const char *fmt, ...)
{
    va_list args;
    int r;
    /* check the log level before do the printk */
    if (level > kernel_current_loglevel)
    {
        return 0;
    }
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);
    return r;
}

char *getloglevel_str(KernelTraceLevel level)
{
    switch (level)
    {
        case KernelNone:   	return "NONE    ";
        case KernelError:   return "ERROR   ";
        case KernelWarning: return "WARNING ";
        case KernelNotice:  return "NOTICE  ";
        case KernelInfo:    return "INFO    ";
        case KernelDebug:   return "DEBUG   ";
        case KernelDebug2:  return "DEBUG2  ";
        case KernelDebug3:  return "DEBUG3  ";
        default:        	return "NO LEVEL";
    }
}

// This function is used to change the current PF_RING debug level, this
// function can be called from userspace applications to change debug level
// dynamically.
void setloglevel(KernelTraceLevel level)
{
	KernelTraceLevel old;
	old = kernel_current_loglevel;
	kernel_current_loglevel = level ;
	printk("kernel log level is changed from %s to %s\n", getloglevel_str(old), getloglevel_str(level));
}


