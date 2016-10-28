#include <stdio.h>
#include <syslog.h>
#include <strings.h>
#include <stdarg.h>

static char id[64];

void init()
{
	bzero(id, sizeof id);
	sprintf(id, "%s", "test");

	openlog(id, LOG_PID, LOG_MAIL);
	
	return;
}

void logs(int level, char* fmt, ...)
{
	char buf[1024];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);
	syslog(level, "%s", buf);
	va_end(args);
}

void vlogs(int level, char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsyslog(LOG_DAEMON | level, fmt, args);
	va_end(args);
}

void deinit()
{
	closelog();
}

int main()
{
	init();
	logs(LOG_DEBUG, "debug message\n");
	logs(LOG_INFO, "info message\n");
	logs(LOG_NOTICE, "notice message\n");
	logs(LOG_WARNING, "warning message\n");
	logs(LOG_ERR, "err message\n");

	vlogs(LOG_DEBUG, "I'm %s\n", "test_rsyslog");

	deinit();

}

/*
*. all the message report to /var/log/syslog  with header "2013-07-26 11:12:41 test[pid]:", "test" is the tagname
1. will reprot to mail.log, mail.debug
2. will reprot to mail.log, mmail.debug mail.info
3. will reprot to mail.log, mail.debug mail.info, mail.notice
4. will reprot to mail.log, mail.debug mail.info, mail.notice, mail.warn
5. will reprot to mail.log, mail.debug mail.info, mail.notice, mail.warn, mail.err
6. will reprot to daemon.log, daemon.debug, test.log 
*/

