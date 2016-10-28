#ifndef _KERNEL_LOG_H_
#define _KERNEL_LOG_H_

/* Debug level */
typedef enum
{
    KernelNone=0,
    KernelError,
    KernelWarning,
    KernelNotice,
    KernelInfo,
    KernelDebug,
    KernelDebug2,
    KernelDebug3
} KernelTraceLevel;

int uprintk(KernelTraceLevel level, const char *fmt, ...);
void setloglevel(KernelTraceLevel level);
char *getloglevel_str(KernelTraceLevel level);

#define KERNEL_LOGGER_ERR(fmt, ...) \
    do{ uprintk(KernelError,   "[KERNEL_LOG] Error:" fmt, ## __VA_ARGS__); }while(0)
#define KERNEL_LOGGER_WARNING(fmt, ...) \
    do{ uprintk(KernelWarning, "[KERNEL_LOG] WARNING:" fmt, ## __VA_ARGS__); }while(0)
#define KERNEL_LOGGER_NOTICE(fmt, ...) \
    do{ uprintk(KernelNotice,  "[KERNEL_LOG] NOTICE :" fmt, ## __VA_ARGS__); }while(0)
#define KERNEL_LOGGER_INFO(fmt, ...) \
    do{ uprintk(KernelInfo,    "[KERNEL_LOG] INFO   :" fmt, ## __VA_ARGS__); }while(0)
#define KERNEL_LOGGER_DEBUG(fmt, ...) \
    do{ uprintk(KernelDebug,   "[KERNEL_LOG] DEBUG  :" fmt, ## __VA_ARGS__); }while(0)
#define KERNEL_LOGGER_DEBUG2(fmt, ...) \
    do{ uprintk(KernelDebug2,  "[KERNEL_LOG] DEBUG2 :" fmt, ## __VA_ARGS__); }while(0)
#define KERNEL_LOGGER_DEBUG3(fmt, ...) \
    do{ uprintk(KernelDebug3,  "[KERNEL_LOG] DEBUG3 :" fmt, ## __VA_ARGS__); }while(0)

#endif
