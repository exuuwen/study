#ifndef __FILE_DEVICE_H_
#define __FILE_DEVICE_H_

#include "ComLogDevice.h"

class FileDevice : public LogDevice
{
public:
	FileDevice(const char * moduleName);
	~FileDevice();
    void write(const char * data, size_t dataLen);

private:
    char* fileName_;
	int fd_;
};

#endif
