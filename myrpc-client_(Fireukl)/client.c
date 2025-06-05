#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <pwd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../libmysyslog1/libmysyslog.h"

#define MAX_BUF 1024
#define HEADER_SIZE 2

void show_usage() {
    printf("Usage: myrpc-client -h HOST -p PORT -c \"COMMAND\" [OPTIONS]\n");
    printf("Options:\n");
    printf("  -c, --command CMD       Command to run (required)\n");
    printf("  -h, --host HOST         Server IP (required)\n");
    printf("  -p, --port PORT         Server port (required)\n");
    printf("  -s, --stream            Use stream socket (default)\n");
    printf("  -d, --dgram             Use datagram socket\n");
    printf("      --help              Show this help\n");
}

int check_response_format(const char *resp) {
    if (strlen(resp) < HEADER_SIZE) {
        return 0;
    }

    if (resp[0] != '0' && resp[0] != '1') {
        return 0;
    }

    if (resp[1] != ':') {
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    char *cmd = NULL;
    char *host = NULL;
    int port = 0;
    int is_tcp = 1;
    int opt;

    static struct option opts[] = {
        {"command", required_argument, 0, 'c'},
        {"host", required_argument, 0, 'h'},
        {"port", required_argument, 0, 'p'},
        {"stream", no_argument, 0, 's'},
        {"dgram", no_argument, 0, 'd'},
        {"help", no_argument, 0, 0},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "c:h:p:sd", opts, NULL)) != -1) {
        switch (opt) {
            case 'c':
                cmd = strdup(optarg);
                break;
            case 'h':
                host = strdup(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                if (port <= 0 || port > 65535) {
                    fprintf(stderr, "Invalid port\n");
                    return 1;
                }
                break;
            case 's':
                is_tcp = 1;
                break;
            case 'd':
                is_tcp = 0;
                break;
            case 0:
                show_usage();
                return 0;
            default:
                show_usage();
                return 1;
        }
    }

    if (!cmd || !host || port == 0) {
        fprintf(stderr, "Error: Missing arguments\n");
        show_usage();

        if (cmd) free(cmd);
        if (host) free(host);
        return 1;
    }

    struct passwd *pw = getpwuid(getuid());
    if (!pw) {
        fprintf(stderr, "Error: Can't get user\n");
        free(cmd);
        free(host);
        return 1;
    }
    const char *user = pw->pw_name;

    char req_buf[MAX_BUF];
    int req_len = snprintf(req_buf, MAX_BUF, "%s:%s", user, cmd);
    if (req_len >= MAX_BUF) {
        fprintf(stderr, "Error: Command too big\n");
        free(cmd);
        free(host);
        return 1;
    }

    mysyslog("Connecting...", INFO, 0, 0, "/var/log/myrpc.log");

    int sock = socket(AF_INET, is_tcp ? SOCK_STREAM : SOCK_DGRAM, 0);
    if (sock < 0) {
        mysyslog("Socket error", ERROR, 0, 0, "/var/log/myrpc.log");
        perror("Error");
        free(cmd);
        free(host);
        return 1;
    }

    struct sockaddr_in srv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port)
    };

    if (inet_pton(AF_INET, host, &srv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid host\n");
        close(sock);
        free(cmd);
        free(host);
        return 1;
    }

    if (is_tcp) {
        if (connect(sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) {
            mysyslog("Connect error", ERROR, 0, 0, "/var/log/myrpc.log");
            perror("Error");
            close(sock);
            free(cmd);
            free(host);
            return 1;
        }

        if (send(sock, req_buf, strlen(req_buf), 0) < 0) {
            mysyslog("Send error", ERROR, 0, 0, "/var/log/myrpc.log");
            perror("Error");
            close(sock);
            free(cmd);
            free(host);
            return 1;
        }

        char reply[MAX_BUF] = {0};
        int reply_len = recv(sock, reply, MAX_BUF - 1, 0);
        if (reply_len < 0) {
            mysyslog("Receive error", ERROR, 0, 0, "/var/log/myrpc.log");
            perror("Error");
            close(sock);
            free(cmd);
            free(host);
            return 1;
        }
        reply[reply_len] = '\0';

        if (!check_response_format(reply)) {
            printf("Bad response: %s\n", reply);
            close(sock);
            free(cmd);
            free(host);
            return 1;
        }

        int code = reply[0] - '0';
        const char *output = reply + 2;

        if (code == 0) {
            printf("%s\n", output);
        } else {
            fprintf(stderr, "Error: %s\n", output);
        }
    } else {
        if (sendto(sock, req_buf, strlen(req_buf), 0,
                  (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) {
            mysyslog("Send error", ERROR, 0, 0, "/var/log/myrpc.log");
            perror("Error");
            close(sock);
            free(cmd);
            free(host);
            return 1;
        }

        char reply[MAX_BUF] = {0};
        socklen_t addr_len = sizeof(srv_addr);
        int reply_len = recvfrom(sock, reply, MAX_BUF - 1, 0,
                               (struct sockaddr*)&srv_addr, &addr_len);
        if (reply_len < 0) {
            mysyslog("Receive error", ERROR, 0, 0, "/var/log/myrpc.log");
            perror("Error");
            close(sock);
            free(cmd);
            free(host);
            return 1;
        }
        reply[reply_len] = '\0';

        if (!check_response_format(reply)) {
            printf("Bad response: %s\n", reply);
            close(sock);
            free(cmd);
            free(host);
            return 1;
        }

        int code = reply[0] - '0';
        const char *output = reply + 2;

        if (code == 0) {
            printf("%s\n", output);
        } else {
            fprintf(stderr, "Error: %s\n", output);
        }
    }

    mysyslog("Command done", INFO, 0, 0, "/var/log/myrpc.log");

    close(sock);
    free(cmd);
    free(host);
    return 0;
}