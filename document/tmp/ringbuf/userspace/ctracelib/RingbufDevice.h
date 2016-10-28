#ifndef __RINGBUF_DEVICE_H_
#define __RINGBUF_DEVICE_H_

#include "ComLogDevice.h"

class RingbufDevice : public LogDevice
{
public:
	 RingbufDevice(const char *name);
	~ RingbufDevice();
    void write(const char * data, size_t dataLen);

private:
	int openDevice(const char *name);
	int fd_;
};

#endif
