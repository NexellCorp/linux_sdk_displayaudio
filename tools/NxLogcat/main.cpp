//------------------------------------------------------------------------------
//
//  Copyright (C) 2016 Nexell Co. All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Name : nxlogcat
//  Description : Nexell log output tool.
//  Author : Chris Lee.
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <sched.h>
#include <regex.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/inotify.h>

#include <NX_Log.h>

#define NXLOGCAT_VERSION	"0.1.0"

#define SYSLOG_D_PID		"/var/run/rsyslogd.pid"
#define SYS_LOG_PATH		"/var/log/syslog"
#define DEBUG_LOG_PATH		"/var/log/debug"
#define USER_LOG_PATH		"/var/log/user.log"
#define MESSAGE_LOG_PATH	"/var/log/messages"

#define CLEAR_COMMAND		">"

#define EVENT_SIZE			(sizeof(struct inotify_event))
#define PRINT_LEN			1024
#define SYSLOG_LEN			2097
#define BUF_LEN				(PRINT_LEN * (EVENT_SIZE + SYSLOG_LEN))

#define MAX_LOG_ROTATE		10

const char* g_option = "hvctpfxl:";
int32_t g_log_level = NXLOG_VERBOSE;
bool g_printTime = false;
bool g_enableFilter = false;
bool g_printPID = false;
bool g_printSys = true;
bool g_printFull = true;
char g_log_level_pattern[NXLOG_LEVEL_COUNT][4];

char verbose_level[4] = "/V ";
char debug_level[4] = "/D ";
char information_level[4] = "/I ";
char notify_level[4] = "/N ";
char warning_level[4] = "/W ";
char error_level[4] = "/E ";

static void print_usage(void) {
	printf("Usage:\n"
	"  On target => nxlogcat or logcat <option> <value>\n"
	"  ex> nxlogcat -t -p -l 5\n"
	"  On pc     => adb logcat or adb shell logcat or adb shell nxlogcat <option> <value>\n"
	"  ex> adb logcat -t -p -l 5\n\n"
	"Common options:\n"
	"  -h: help\n"
	"  -v: print versions (macro header, this program)\n\n"
	"Print options:\n"
	"  -c: clear the log\n"
	"  -t: print with time\n"
	"  -p: print with PID\n"
	"  -f: print only the current log\n"
	"  -x: print without system log (kernel, systemd, etc...)\n"
	"  -l <level>: print the log up to the specified level\n\n"
	"Level:\n"
	"  0 : error\n"
	"  1 : warning\n"
	"  2 : notice\n"
	"  3 : information\n"
	"  4 : debug\n"
	"  5 : verbose\n\n");
}

static void print_version(void) {
	printf("nxlogcat version %s\n", NXLOGCAT_VERSION);
	printf("nxlog macro header version %s\n", NXLOG_VERSION);
}

static int32_t logcat_clear(const char *logPath) {
	char command[30];

	memset(command, 0, sizeof(command));
	sprintf(command, "%s %s", CLEAR_COMMAND, logPath);
	return system(command);
}

static void logcat_clear_all(void) {
	char clearPath[30];

	if (access(SYS_LOG_PATH, F_OK) == 0) {
		logcat_clear(SYS_LOG_PATH);
		logcat_clear(DEBUG_LOG_PATH);
		logcat_clear(USER_LOG_PATH);
		logcat_clear(MESSAGE_LOG_PATH);
	}

	for (int32_t rotate = 1; rotate <= MAX_LOG_ROTATE; rotate++) {
		memset(clearPath, 0, sizeof(clearPath));
		sprintf(clearPath, "%s.%d", SYS_LOG_PATH, rotate);
		if (access(clearPath, F_OK) == 0) {
			logcat_clear(clearPath);
		}
	}
}

#if 0
static int32_t logcat_setlogmask(int32_t mask_level) {
	return setlogmask(LOG_UPTO(mask_level + 3));
}
#endif

static void nxlogcat_parser(char *line) {
	char tmp1[PRINT_LEN], tmp2[PRINT_LEN], tmp3[PRINT_LEN], nxlog[PRINT_LEN];
	char *date, *proc, proc_p[PRINT_LEN], *msg;
	bool is_not_pid = false;
	bool is_not_loglevel = false;

    memset(tmp1, 0, sizeof(tmp1));
    memset(tmp2, 0, sizeof(tmp2));
    memset(tmp3, 0, sizeof(tmp3));
    memset(nxlog, 0, sizeof(nxlog));
    memset(proc_p, 0, sizeof(proc_p));

    strncpy(tmp1, line, sizeof(tmp1));
    strncpy(tmp2, line, sizeof(tmp2));
    strncpy(tmp3, line, sizeof(tmp3));

    date = strtok(tmp1, " ");

    proc = strtok(tmp2, " ");
    proc = strtok(NULL, " ");
    proc = strtok(NULL, ":");

    if (proc[strlen(proc) - 1] == ']') {
        for (int i = strlen(proc) - 2; i >= 0; i--) {
            if (proc[i] == '[') {
                strncpy(proc_p, proc, i);
                break;
            }
        }
		// Regex expression
		regex_t t_regex;
        regcomp(&t_regex, "/", REG_EXTENDED);
        if (regexec(&t_regex, proc_p, 0, NULL, 0)) {
            is_not_loglevel = true;
        }
        regfree(&t_regex);
    } else {
        is_not_pid = true;
    }

    msg = strtok(tmp3, " ");
    msg = strtok(NULL, " ");
    if (is_not_pid == false) {
        if (is_not_loglevel != true) {
            msg = strtok(NULL, " ");
        }
    }
    msg = strtok(NULL, ":");
    msg = strtok(NULL, "");

    if (is_not_pid || g_printPID) {
        if (g_printTime) {
	            snprintf(nxlog, sizeof(nxlog), "%s %s:%s", date, proc, msg);
        } else {
	            snprintf(nxlog, sizeof(nxlog), "%s:%s", proc, msg);
        }
    } else {
        if (g_printTime) {
	            snprintf(nxlog, sizeof(nxlog), "%s %s:%s", date, proc_p, msg);
        } else {
	            snprintf(nxlog, sizeof(nxlog), "%s:%s", proc_p, msg);
        }
    }

    if (g_enableFilter) {
        // Regex expression
        regex_t f_regex;
        regcomp(&f_regex, "/", REG_EXTENDED);
		if (is_not_pid || g_printPID) {
	        if (regexec(&f_regex, proc, 0, NULL, 0)) {
				if (g_printSys) {
					printf("%s", nxlog);
				}
	        }
	        regfree(&f_regex);
		} else {
	        if (regexec(&f_regex, proc_p, 0, NULL, 0)) {
				if (g_printSys) {
					printf("%s", nxlog);
				}
	        }
	        regfree(&f_regex);
		}

        for (int i = 0; i <= g_log_level; i++) {
	        // Regex expression
	        regex_t l_regex;
            regcomp(&l_regex, g_log_level_pattern[i], REG_EXTENDED);
            if (!regexec(&l_regex, nxlog, 0, NULL, 0)) {
                printf("%s", nxlog);
            }
            regfree(&l_regex);
        }
    } else {
        // Regex expression
        regex_t s_regex;
        regcomp(&s_regex, "/", REG_EXTENDED);
        if (is_not_pid || g_printPID) {
            if (regexec(&s_regex, proc, 0, NULL, 0)) {
                if (g_printSys) {
                    printf("%s", nxlog);
                }
            } else {
				printf("%s", nxlog);
			}
            regfree(&s_regex);
        } else {
            if (regexec(&s_regex, proc_p, 0, NULL, 0)) {
                if (g_printSys) {
                    printf("%s", nxlog);
                }
            } else {
				printf("%s", nxlog);
			}
            regfree(&s_regex);
        }
	}
}

static void logcat(FILE *log_file) {
	char line[PRINT_LEN];

	// Print the log
	while (1) {
		memset(line, 0, sizeof(line));
		if (!fgets(line, sizeof(line) - 1, log_file))
			break;
		nxlogcat_parser(line);
	}
}

int32_t main(int32_t argc, char *argv[]) {
	FILE *hFile = NULL;
	FILE *bFile = NULL;
	int32_t opt;
	int32_t iNotify = -1;
	int32_t iWatch = -1;
//	int32_t mask_level;
	int32_t rotate;
	char readEvent[BUF_LEN];
	char backupFile[30];
	struct sched_param param;
	pid_t pid;
	fpos_t pos;

	// Phase 0 : Set schedule priority
	memset(&param, 0, sizeof(param));
	pid = getpid();
	sched_setscheduler(pid, SCHED_BATCH, &param);

	// Phase 1 : Parse command options
	while ((opt = getopt(argc, argv, g_option)) != -1) {
		switch(opt) {
			case 'h':
			case '?':
				print_usage();
				return 0;
			case 'v':
				print_version();
				return 0;
			case 'c':
				logcat_clear_all();
				return 0;
			case 't':
				g_printTime = true;
				break;
			case 'p':
				g_printPID = true;
				break;
			case 'f':
				g_printFull = false;
				break;
			case 'x':
				g_printSys = false;
				break;
#if 0
			case 's':
				mask_level = atoi(optarg);
				if (mask_level < 0 || mask_level > NXLOG_VERBOSE)
					mask_level = NXLOG_VERBOSE;
				logcat_setlogmask(mask_level);
				return 0;
#endif
			case 'l':
				g_enableFilter = true;
				g_log_level = atoi(optarg);
				if (g_log_level < 0 || g_log_level > NXLOG_VERBOSE)
					g_log_level = NXLOG_VERBOSE;

				// Initialize the log level pattern
				strncpy(g_log_level_pattern[NXLOG_VERBOSE], verbose_level, sizeof(verbose_level));
				strncpy(g_log_level_pattern[NXLOG_DEBUG], debug_level, sizeof(debug_level));
				strncpy(g_log_level_pattern[NXLOG_INFO], information_level, sizeof(information_level));
				strncpy(g_log_level_pattern[NXLOG_NOTICE], notify_level, sizeof(notify_level));
				strncpy(g_log_level_pattern[NXLOG_WARNING], warning_level, sizeof(warning_level));
				strncpy(g_log_level_pattern[NXLOG_ERR], error_level, sizeof(error_level));
				break;
			default:
				printf("getopt returned character code 0%o\n", opt);
		}
	}

	if (optind < argc) {
		while (optind < argc) {
			optind += 1;
		}
	}

	// Phase 2 : Check if syslog daemon is running
	if (access(SYSLOG_D_PID, F_OK) < 0) {
		printf("Please check if syslog daemon is running.\n");
		goto ERROR;
	}

	// Phase 3 : Open the log file
	if ((hFile = fopen(SYS_LOG_PATH, "r")) == NULL) {
		printf("Fail, fopen(). (%s)\n", strerror(errno));
		printf("Can't find syslog file. (%s)\n"
		"Please check if syslog daemon is running.\n", SYS_LOG_PATH);
		goto ERROR;
	}

	memset(backupFile, 0, sizeof(backupFile));

	// Phase 4 : Print the log at first
	if (g_printFull) {
		// Print the backup log files
		for (rotate = MAX_LOG_ROTATE; rotate > 1; rotate--) {
			sprintf(backupFile, "%s.%d", SYS_LOG_PATH, rotate);
			if (access(backupFile, F_OK) == 0) {
				if ((bFile = fopen(backupFile, "r")) != NULL) {
					logcat(bFile);
				}
			}

			if (bFile != NULL) {
				fclose(bFile);
			}
		}

		memset(backupFile, 0, sizeof(backupFile));
		sprintf(backupFile, "%s.%d", SYS_LOG_PATH, rotate);
		if (access(backupFile, F_OK) == 0) {
			if ((bFile = fopen(backupFile, "r")) != NULL) {
				logcat(bFile);
			}
		}

		if (bFile != NULL) {
			fclose(bFile);
		}
	}

	logcat(hFile);

	// Phase 5 : Set notify event
	iNotify = inotify_init();
	if (iNotify < 0) {
		printf("Fail, inotify_init(). (%s)\n", strerror(errno));
		goto ERROR;
	}

	// Phase 6 : Watch the notification event
	iWatch = inotify_add_watch(iNotify, SYS_LOG_PATH, IN_MODIFY | IN_DELETE_SELF | IN_MOVE_SELF);
	if (iWatch < 0) {
		printf("Fail, inotify_add_watch(). (%s)\n", strerror(errno));
		goto ERROR;
	}

	// Phase 7 : Polling
	while (1) {
		struct pollfd hPoll;
		hPoll.fd		= iNotify;
		hPoll.events	= POLLIN | POLLERR;
		hPoll.revents	= 0;
		int32_t ret = poll((struct pollfd *)&hPoll, 1, 1000);
		if (ret < 0) {
			printf("Fail, poll(). (%s)\n", strerror(errno));
			break;
		} else if (ret == 0) {
			continue;
		}

		// Phase 8 : Read
		if (hPoll.revents & POLLIN) {
			int32_t readSize = read(iNotify, readEvent, BUF_LEN);
			int32_t eventPos = 0;
			if (readSize < 0) {
				printf("Fail, read(). (%s)\n", strerror(errno));
				break;
			}

			// Phase 9 : Detect notification event
			while (eventPos < readSize) {
				struct inotify_event *pNotifyEvent = (struct inotify_event *)&readEvent[eventPos];
				if (pNotifyEvent->mask & IN_MODIFY) {
					fgetpos(hFile, &pos);
					fseek(hFile, 0, SEEK_END);
					int32_t fileLength = ftell(hFile);
					fsetpos(hFile, &pos);

					// Re-open log file
					if (fileLength == 0) {
						fclose(hFile);
						if ((hFile = fopen(SYS_LOG_PATH, "r")) == NULL) {
							printf("Fail, fopen(). (%s)\n", strerror(errno));
							printf("Can't find syslog file. (%s)\n"
							"Please check if syslog daemon is running.\n", SYS_LOG_PATH);
							goto ERROR;
						}
					}

					// Phase 10 : Print the log continously
					logcat(hFile);
				} else if (pNotifyEvent->mask & IN_DELETE_SELF || pNotifyEvent->mask & IN_MOVE_SELF) {
					break;
				}
				eventPos += EVENT_SIZE + pNotifyEvent->len;
			}
		}
	}

ERROR:
	if (iWatch > 0)
		inotify_rm_watch(iNotify, iWatch);
	if (iNotify > 0)
		close(iNotify);
	if (hFile)
		fclose(hFile);

	return 0;
}
