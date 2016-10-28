#include "ComTraceType.h"
#include "ComTraceHdrOption.h"
#include "RsyslogDevice.h"

#include <syslog.h>

RsyslogDevice::RsyslogDevice()
	: LogDevice(cttRsyslog)
{
	defHdrOptions_ = logModuleName | logTraceLevel;
}

RsyslogDevice::~RsyslogDevice()
{
	printf("in %s\n", __func__);
}

void RsyslogDevice::write(const char *data, size_t dataLen)
{
    syslog(0, "%s", data);
}





