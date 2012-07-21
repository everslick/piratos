#ifndef _SYS_LOG_H_
#define _SYS_LOG_H_

enum {
	SYS_LOG_NEVER,
	SYS_LOG_FATAL,
	SYS_LOG_ERROR,
	SYS_LOG_WARN,
	SYS_LOG_INFO,
	SYS_LOG_DEBUG,
	SYS_LOG_ALWAYS
};

void syslog(int level, const char *fmt, ...);

#endif // _SYS_LOG_H_
