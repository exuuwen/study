#include "ComCTrace.h"

#include <pthread.h>
#include <cstdio>
#include <unistd.h>
#include <cassert>

ComTraceHdrOption hdrOptions = logModuleName | logTraceLevel | logFileAndLine | logTime;
ComCTrace* test_ringbufDevice = CT_create("ringbuflog_test", cttRingbuflog, CtlDebug); 

#define TEST_LOGGER_DEBUG3(fmt...)   CT_debug3(test_ringbufDevice, fmt)
#define TEST_LOGGER_DEBUG2(fmt...)   CT_debug2(test_ringbufDevice, fmt)
#define TEST_LOGGER_DEBUG(fmt...)    CT_debug(test_ringbufDevice, fmt)
#define TEST_LOGGER_INFO(fmt...)     CT_info(test_ringbufDevice, fmt)
#define TEST_LOGGER_NOTICE(fmt...)   CT_notice(test_ringbufDevice, fmt)
#define TEST_LOGGER_WARNING(fmt...)  CT_warning(test_ringbufDevice, fmt)
#define TEST_LOGGER_ERROR(fmt...)    CT_error(test_ringbufDevice, fmt)

void* func1(void *d)
{
	TEST_LOGGER_DEBUG3("func 1 hahah debug3\n");
	TEST_LOGGER_DEBUG2("func 1 hahah debug2\n");
	TEST_LOGGER_DEBUG("func 1 hahah debug\n");
	sleep(2);
	TEST_LOGGER_INFO("func 1 hahah info\n");
	TEST_LOGGER_NOTICE("func 1 hahah notice\n");
	TEST_LOGGER_WARNING("func 1 hahah warning\n");
	TEST_LOGGER_ERROR("func 1 hahah error\n");
	
}

void* func2(void *d)
{
	TEST_LOGGER_DEBUG3("func 2 hahah debug3\n");
	TEST_LOGGER_DEBUG2("func 2 hahah debug2\n");
	TEST_LOGGER_DEBUG("func 2 hahah debug\n");
	TEST_LOGGER_INFO("func 2 hahah info\n");
	sleep(2);
	TEST_LOGGER_NOTICE("func 2 hahah notice\n");
	TEST_LOGGER_WARNING("func 2 hahah warning\n");
	TEST_LOGGER_ERROR("func 2 hahah error\n");
	
}


int main()
{
	pthread_t p1, p2;

	int ret = pthread_create(&p1, NULL, func1, NULL);
	assert(ret == 0);
	ret = pthread_create(&p2, NULL, func2, NULL);
	assert(ret == 0);

	sleep(2);
	TEST_LOGGER_DEBUG3("first main hahah debug3\n");
	TEST_LOGGER_DEBUG2("first main hahah debug2\n");
	TEST_LOGGER_DEBUG("first main hahah debug\n");
	TEST_LOGGER_INFO("first main hahah info\n");
	TEST_LOGGER_NOTICE("first main hahah notice\n");
	TEST_LOGGER_WARNING("first main hahah warning\n");
	TEST_LOGGER_ERROR("first main hahah error\n");
	
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);

	CT_setHeaderOptions(test_ringbufDevice, hdrOptions);
	CT_setLevel(test_ringbufDevice, CtlDebug3);

	sleep(2);
	TEST_LOGGER_DEBUG3("main hahah debug3\n");
	TEST_LOGGER_DEBUG2("main hahah debug2\n");
	TEST_LOGGER_DEBUG("main hahah debug\n");
	TEST_LOGGER_INFO("main hahah info\n");
	TEST_LOGGER_NOTICE("main hahah notice\n");
	TEST_LOGGER_WARNING("main hahah warning\n");
	TEST_LOGGER_ERROR("main hahah error\n");

	CT_destroy(test_ringbufDevice);

	return 0; 
}
