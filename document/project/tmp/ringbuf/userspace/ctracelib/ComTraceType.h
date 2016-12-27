#ifndef _COM_C_TRACE_TYPE_H_
#define _COM_C_TRACE_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	cttFilelog,
	cttRsyslog,
    cttRingbuflog	
} ComTraceType;

#ifdef __cplusplus
}
#endif

#endif
