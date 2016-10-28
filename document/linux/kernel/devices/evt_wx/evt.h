/****************************************************************
*
* FILENAME
*   evt.h
*
* PURPOSE
*   
*
* AUTHOR
*   Dragontec
*
* DATE
*  9 Nov 2010
*
* HISTORY
*   2010.11.09  Rev.1.0  1st revision
*
****************************************************************/
#ifndef _EVT_H_
#define _EVT_H_

/****************************************************************/
/*          MACROS/DEFINES                                      */
/****************************************************************/
typedef unsigned int DEV_EVT_CODE; 

/****************************************************************/
/*          LOCAL TYPES                                         */
/****************************************************************/
typedef enum{
	EVT_PLUG = 0,
	EVT_SIGNAL,
	EVT_SYS,
	EVT_TYPE_MAX
}DEV_EVT_TYPE;

typedef enum
{
	DEV_EVT_OK,
	DEV_EVT_NOT_INITIALISED,
	DEV_EVT_INVALID_PARAM,
	DEV_EVT_ALREADY_REGISTERED,
	DEV_EVT_FAIL,
	DEV_EVT_NOT_SUPPORTED,
	DEV_EVT_MEMORY_ALLOC_FAIL
}DEV_EVT_RESULT_CODE;

/****************************************************************/
/*          PUBLIC FUNCTIONS                                    */
/****************************************************************/
extern DEV_EVT_RESULT_CODE dev_evt_RegEvt(DEV_EVT_TYPE event_type,int event_code,int event_value,DEV_EVT_CODE* code);
extern DEV_EVT_RESULT_CODE dev_evt_UnRegEvt(DEV_EVT_CODE code);
extern DEV_EVT_RESULT_CODE dev_evt_InjectEvt(DEV_EVT_CODE code);

#endif /*_EVT_H_*/
