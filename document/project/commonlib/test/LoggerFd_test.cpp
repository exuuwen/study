#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Logger.h"

int main(int argc, char *argv[])
{	
	
	FdLogger logger(0);

	TraceLevel level = logger.getLogLevel();
	printf("orginal log level %d\n", level);

	logger.setLogLevel(Notice);
	level = logger.getLogLevel();
	printf("after log level %d\n", level);

	logger.log(Warning, "file level %d", level);
	
	logger.log(Notice, "my id %d", 55);
	logger.log(Info, "file level %d", level);
	
	int fd;
	
	fd = open("fdlogger.log", O_RDWR | O_CREAT, 0660);
	if (fd < 0)
	{
		perror("fd < 0\n");
		assert(0);
	}
	
	bool ret = logger.setLogFd(fd);
	if(ret == false)
	{
		printf("SetLogDirFile error\n");
		assert(0);
	}
	logger.setLogLevel(Info);
	level = logger.getLogLevel();
	printf("again log level %d\n", level);

	logger.log(Warning, "a file level %d", 1);
	logger.log(Notice, "my id %d", 2);
	logger.log(Info, "a file level %d", 3);

	close(fd);

	initFdLogger(0, Notice);

	logWarnMsg("waring %d", 1);
	logNoticeMsg("notice %d", 2);
	logInfoMsg("info %d", 3);

	freeFdLogger();
	
	return 0;
}
