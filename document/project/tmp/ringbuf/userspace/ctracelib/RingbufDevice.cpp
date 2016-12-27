#include "ComTraceType.h"
#include "ComTraceHdrOption.h"
#include "RingbufDevice.h"

#include "../kernel/ringbuf_ioctl.h"

#include <cstdio>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <cassert>

#define RB_DEV_ALL_RAW "/dev/rb_0_raw"

RingbufDevice::RingbufDevice(const char *name)
	: LogDevice(cttRingbuflog)
{
	defHdrOptions_ = logTraceLevel | logModuleName;
	fd_ = openDevice(name);
	assert(fd_ >= 0);
}

RingbufDevice::~RingbufDevice()
{
	::close(fd_);
	printf("in %s\n", __func__);
}

void RingbufDevice::write(const char *data, size_t dataLen)
{
    ::write(fd_, data, dataLen);
}

int RingbufDevice::openDevice(const char *name)
{
	int fd = ::open(RB_DEV_ALL_RAW, 0);
	if (fd < 0)
	{
		perror("open:");
		return -1;
	}
	
	Tag tag;
	bzero(&tag, sizeof(Tag));
	
	strcpy(tag.tag_name, name);
	printf("tag:%s\n", tag.tag_name);

	int ret = ::ioctl(fd, RB_IOCSETTAG, &tag);
	::close(fd);
	if (ret != 0)
	{
		perror("ioctl RB_IOCSETTAG:");
		return -1;
	}

	char dev[20];
	bzero(dev, sizeof(dev));
	sprintf(dev, "/dev/rb_%u_raw", tag.tag_no);
		
	printf("dev name: %s\n", dev);

	fd = ::open(dev, O_RDWR);
	if (fd < 0)
	{
		perror("open2:");
		return -1;
	}

	return fd;	
}

