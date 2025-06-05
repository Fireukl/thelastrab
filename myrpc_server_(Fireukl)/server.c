#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pwd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "config_parser.h"
#include "../libmysyslog1/libmysyslog.h"
#include <sys/types.h>
#include <sys/stat.h>

#define MAX_DATA_SIZE 1024

volatile sig_atomic_t stop_server;

void handle_signal(int sig) {
    stop_server = 1;
}

void daemonize() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    setsid();
    umask(0);

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

int verify_user_access(const char *username) {
    FILE *user_file = fopen("/etc/myRPC/users.conf", "r");
    if (!user_file) {
        mysyslog("Cannot open users.conf", ERROR, 0, 0, "/var/log/myrpc.log");
        perror("Cannot open users.conf");
        return 0;
    }

    char line[256];
    int access_allowed = 0;

    while (fgets(line, sizeof(line), user_file)) {
        line[strcspn(line, "\n")] = '\0';

        if (line[0] == '#' || strlen(line) == 0)
            continue;

        if (strcmp(line, username) == 0) {
            access_allowed = 1;
            break;
        }
    }

    fclose(user_file);
    return access_allowed;
}

int execute_command(const char *command, char *out_file, char *err_file) {
    char full_command[MAX_DATA_SIZE];
    snprintf(full_command, MAX_DATA_SIZE, "%s >%s 2>%s", command, out_file, err_file);
    int status = system(full_command);
    return WEXITSTATUS(status);
}

int main() {
    daemonize();

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    Config cfg = parse_config("/etc/myRPC/myRPC.conf");

    int port = cfg.port;
    int use_tcp = strcmp(cfg.socket_type, "stream") == 0;

    mysyslog("Starting server...", INFO, 0, 0, "/var/log/myrpc.log");

    int sock;
    if (use_tcp) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
    } else {
        sock = socket(AF_INET, SOCK_DGRAM, 0);
    }

    if (sock < 0) {
        mysyslog("Cannot create socket", ERROR, 0, 0, "/var/log/myrpc.log");
        perror("Cannot create socket");
        return 1;
    }

    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        mysyslog("setsockopt error", ERROR, 0, 0, "/var/log/myrpc.log");
        perror("setsockopt error");
        close(sock);
        return 1;
    }

    struct sockaddr_in srv_addr, cli_addr;
    socklen_t addr_size;
    memset(&srv_addr, 0, sizeof(srv_addr));
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) < 0) {
        mysyslog("Bind error", ERROR, 0, 0, "/var/log/myrpc.log");
        perror("Bind error");
        close(sock);
        return 1;
    }

    if (use_tcp) {
        listen(sock, 5);
        mysyslog("Server ready (TCP)", INFO, 0, 0, "/var/log/myrpc.log");
    } else {
        mysyslog("Server ready (UDP)", INFO, 0, 0, "/var/log/myrpc.log");
    }

    while (!stop_server) {
        char buffer[MAX_DATA_SIZE];
        int bytes;

        if (use_tcp) {
            addr_size = sizeof(cli_addr);
            int client_sock = accept(sock, (struct sockaddr*)&cli_addr, &addr_size);
            if (client_sock < 0) {
                mysyslog("Accept error", ERROR, 0, 0, "/var/log/myrpc.log");
                perror("Accept error");
                continue;
            }

            bytes = recv(client_sock, buffer, MAX_DATA_SIZE, 0);
            if (bytes <= 0) {
                close(client_sock);
                continue;
            }
            buffer[bytes] = '\0';

            mysyslog("Request received", INFO, 0, 0, "/var/log/myrpc.log");

            char *user = strtok(buffer, ":");
            char *cmd = strtok(NULL, "");
            if (cmd) {
                while (*cmd == ' ')
                    cmd++;
            }

            char response[MAX_DATA_SIZE];

            if (verify_user_access(user)) {
                mysyslog("Access granted", INFO, 0, 0, "/var/log/myrpc.log");

                char out_tmp[] = "/tmp/myRPC_XXXXXX.stdout";
                char err_tmp[] = "/tmp/myRPC_XXXXXX.stderr";
                mkstemp(out_tmp);
                mkstemp(err_tmp);

                int res = execute_command(cmd, out_tmp, err_tmp);
                response[0] = (res == 0) ? '0' : '1';
                response[1] = ':';

                FILE *out = fopen(out_tmp, "r");
                if (out) {
                    size_t read = fread(response + 2, 1, MAX_DATA_SIZE - 3, out);
                    response[0] = '0';
                    response[1] = ':';
                    response[read + 2] = '\0';
                    fclose(out);
                    mysyslog("Command successful", INFO, 0, 0, "/var/log/myrpc.log");
                } else {
                    strcpy(response, "1:Error reading output");
                    mysyslog("Output read error", ERROR, 0, 0, "/var/log/myrpc.log");
                }

                remove(out_tmp);
                remove(err_tmp);

            } else {
                snprintf(response, MAX_DATA_SIZE, "1:User '%s' denied", user);
                mysyslog("Access denied", WARNING, 0, 0, "/var/log/myrpc.log");
            }

            mysyslog("Sending response", INFO, 0, 0, "/var/log/myrpc.log");
            mysyslog(response, INFO, 0, 0, "/var/log/myrpc.log");
            send(client_sock, response, strlen(response), 0);
            close(client_sock);

        } else {
            addr_size = sizeof(cli_addr);
            bytes = recvfrom(sock, buffer, MAX_DATA_SIZE, 0, (struct sockaddr*)&cli_addr, &addr_size);
            if (bytes <= 0) {
                continue;
            }
            buffer[bytes] = '\0';

            mysyslog("Request received", INFO, 0, 0, "/var/log/myrpc.log");

            char *user = strtok(buffer, ":");
            char *cmd = strtok(NULL, "");
            if (cmd) {
                while (*cmd == ' ')
                    cmd++;
            }

            char response[MAX_DATA_SIZE];

            if (verify_user_access(user)) {
                mysyslog("Access granted", INFO, 0, 0, "/var/log/myrpc.log");

                char out_tmp[] = "/tmp/myRPC_XXXXXX.stdout";
                char err_tmp[] = "/tmp/myRPC_XXXXXX.stderr";
                mkstemp(out_tmp);
                mkstemp(err_tmp);

                int res = execute_command(cmd, out_tmp, err_tmp);
                response[0] = (res == 0) ? '0' : '1';
                response[1] = ':';

                FILE *out = fopen(out_tmp, "r");
                if (out) {
                    size_t read = fread(response + 2, 1, MAX_DATA_SIZE - 3, out);
                    response[0] = '0';
                    response[1] = ':';
                    response[read + 2] = '\0';
                    fclose(out);
                    mysyslog("Command successful", INFO, 0, 0, "/var/log/myrpc.log");
                } else {
                    strcpy(response, "1:Error reading output");
                    mysyslog("Output read error", ERROR, 0, 0, "/var/log/myrpc.log");
                }

                remove(out_tmp);
                remove(err_tmp);

            } else {
                snprintf(response, MAX_DATA_SIZE, "1:User '%s' denied", user);
                mysyslog("Access denied", WARNING, 0, 0, "/var/log/myrpc.log");
            }

            sendto(sock, response, strlen(response), 0, (struct sockaddr*)&cli_addr, addr_size);
        }
    }

    close(sock);
    mysyslog("Server stopped", INFO, 0, 0, "/var/log/myrpc.log");
    return 0;
}
