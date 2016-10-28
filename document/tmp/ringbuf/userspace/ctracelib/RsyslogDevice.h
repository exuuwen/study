#ifndef __RSYSLOG_DEVICE_H_
#define __RSYSLOG_DEVICE_H_

#include "ComLogDevice.h"

class RsyslogDevice : public LogDevice
{
public:
	RsyslogDevice();
	~RsyslogDevice();
    void write(const char * data, size_t dataLen);    
};

#endif
