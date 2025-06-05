#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "libmysyslog.h"

int mysyslog(const char* message, int level, int driver, int format, const char* path) {
    FILE *log_file;
    log_file = fopen(path, "a");
    if (log_file == NULL) {
        return -1;
    }

    time_t now;
    time(&now);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0';

    const char *level_str;
    switch (level) {
        case DEBUG:    level_str = "DEBUG"; break;
        case INFO:     level_str = "INFO"; break;
        case WARNING:  level_str = "WARNING"; break;
        case ERROR:    level_str = "ERROR"; break;
        case CRITICAL: level_str = "CRITICAL"; break;
        default:      level_str = "UNKNOWN"; break;
    }

    if (format == 0) {
        fprintf(log_file, "%s %s %d %s\n", time_str, level_str, driver, message);
    } else {
        fprintf(log_file,
               "{\"timestamp\":\"%s\",\"level\":\"%s\",\"driver\":%d,\"msg\":\"%s\"}\n", time_str, level_str, driver, message);
    }

    fclose(log_file);
    return 0;
}
