#ifndef _RINGBUF_LOG_H_
#define _RINGBUF_LOG_H_

/* Debug level */
typedef enum
{
    RbNone=0,
    RbError,
    RbWarning,
    RbNotice,
    RbInfo,
    RbDebug,
    RbDebug2,
    RbDebug3
} RbTraceLevel;

int uprintk(RbTraceLevel level, const char *fmt, ...);
void setPfloglevel(RbTraceLevel level);
char *getloglevel_str(RbTraceLevel level);

#define RINGBUF_LOGGER_ERR(fmt, ...) \
    do{ uprintk(RbError, "[RINGBUF_LOG] Error:" fmt, ## __VA_ARGS__); }while(0)
#define RINGBUF_LOGGER_WARNING(fmt, ...) \
    do{ uprintk(RbWarning, "[RINGBUF_LOG] WARNING:" fmt, ## __VA_ARGS__); }while(0)
#define RINGBUF_LOGGER_NOTICE(fmt, ...) \
    do{ uprintk(RbNotice,  "[RINGBUF_LOG] NOTICE :" fmt, ## __VA_ARGS__); }while(0)
#define RINGBUF_LOGGER_INFO(fmt, ...) \
    do{ uprintk(RbInfo,    "[RINGBUF_LOG] INFO   :" fmt, ## __VA_ARGS__); }while(0)
#define RINGBUF_LOGGER_DEBUG(fmt, ...) \
    do{ uprintk(RbDebug,   "[RINGBUF_LOG] DEBUG  :" fmt, ## __VA_ARGS__); }while(0)
#define RINGBUF_LOGGER_DEBUG2(fmt, ...) \
    do{ uprintk(RbDebug2,  "[RINGBUF_LOG] DEBUG2 :" fmt, ## __VA_ARGS__); }while(0)
#define RINGBUF_LOGGER_DEBUG3(fmt, ...) \
    do{ uprintk(RbDebug3,  "[RINGBUF_LOG] DEBUG3 :" fmt, ## __VA_ARGS__); }while(0)

#endif
