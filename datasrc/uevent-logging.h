#ifndef _UEVENT_LOGGING_H_
#define _UEVENT_LOGGING_H_

#include <syslog.h>
#include <stdlib.h>

void logging_init(void);
void logging_close(void);
int logging_level(const char *lvl);

void message(int priority, const char *fmt, ...);

#define fatal(format, arg...)                                           \
	do {                                                            \
		message(LOG_CRIT,                                       \
			"%s: %s(%d): %s: " format ,                     \
			PROGRAM_NAME, __FILE__, __LINE__, __FUNCTION__, \
			## arg);                                        \
		exit(EXIT_FAILURE);                                     \
	} while (0)

#define err(format, arg...)                               \
	do {                                              \
		message(LOG_ERR,                          \
			"%s: " format ,                   \
			PROGRAM_NAME,                     \
			## arg);                          \
	} while (0)

#define info(format, arg...)                              \
	do {                                              \
		message(LOG_INFO,                         \
			"%s: " format ,                   \
			PROGRAM_NAME,                     \
			## arg);                          \
	} while (0)

#define dbg(format, arg...)                                             \
	do {                                                            \
		message(LOG_DEBUG,                                      \
			"%s: %s(%d): %s: " format ,                     \
			PROGRAM_NAME, __FILE__, __LINE__, __FUNCTION__, \
			## arg);                                        \
	} while (0)

#endif /* _UEVENT_LOGGING_H_ */
