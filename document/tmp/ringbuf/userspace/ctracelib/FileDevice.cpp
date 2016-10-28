#include "FileDevice.h"
#include "ComTraceType.h"
#include "ComTraceHdrOption.h"

#include <cassert>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>

#define LOG "log"

FileDevice::FileDevice(const char* name) 
		: LogDevice(cttFilelog)
{
	
	fileName_ = new char[std::strlen(name) + sizeof(LOG)*2 + 1];
    sprintf(fileName_, "%s/%s.%s", LOG, name, LOG);

	fd_ = ::open(fileName_, O_CREAT | O_APPEND | O_RDWR, 0644);
	if (fd_ < 0)
	{
		perror("fd fail:");
		assert(0);
	}
	
	defHdrOptions_ = logModuleName | logTraceLevel | logTime | logProcessName;
}

void FileDevice::write(const char * data, size_t dataLen)
{
    ::write(fd_, data, dataLen);
}

FileDevice::~FileDevice()
{
	printf("in %s\n", __func__);
	delete fileName_;
	close(fd_);
}

