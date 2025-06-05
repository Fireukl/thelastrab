#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config_parser.h"

Config parse_config(const char *config_file) {
    Config conf;
    conf.port = 0;
    strcpy(conf.socket_type, "stream");

    FILE *file = fopen(config_file, "r");
    if (!file) {
        perror("Cannot open config file");
        return conf;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '#' || strlen(line) == 0)
            continue;

        char *key = strtok(line, "=");
        char *val = strtok(NULL, "");

        if (strcmp(key, "port") == 0) {
            conf.port = atoi(val);
        } else if (strcmp(key, "socket_type") == 0) {
            strcpy(conf.socket_type, val);
        }
    }

    fclose(file);
    return conf;
}