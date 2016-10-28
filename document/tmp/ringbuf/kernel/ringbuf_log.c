#include "ringbuf_log.h"

#include <linux/time.h>

RbTraceLevel rb_current_loglevel = RbDebug3;

// Kernel logger wrapper
// Implemented a debug level based mechnism
int uprintk(RbTraceLevel level, const char *fmt, ...)
{
    va_list args;
    int r;
    /* check the log level before do the printk */
    if (level > rb_current_loglevel)
    {
        return 0;
    }
    va_start(args, fmt);
    r = vprintk(fmt, args);
    va_end(args);
    return r;
}

char *getloglevel_str(RbTraceLevel level)
{
    switch (level)
    {
        case RbNone:   return "NONE    ";
        case RbError:   return "ERROR   ";
        case RbWarning: return "WARNING ";
        case RbNotice:  return "NOTICE  ";
        case RbInfo:    return "INFO    ";
        case RbDebug:   return "DEBUG   ";
        case RbDebug2:  return "DEBUG2  ";
        case RbDebug3:  return "DEBUG3  ";
        default:        return "NO LEVEL";
    }
}

// This function is used to change the current PF_RING debug level, this
// function can be called from userspace applications to change debug level
// dynamically.
void setPfloglevel(RbTraceLevel level)
{
	RbTraceLevel old;
	old = rb_current_loglevel;
	rb_current_loglevel = level ;
	printk("pfring log level is changed from %s to %s\n", getloglevel_str(old), getloglevel_str(level));
}

