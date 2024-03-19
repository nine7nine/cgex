#include "libcgex.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <dirent.h>
#include <string.h>

#define DAEMON_SOCKET_PATH "/var/run/cgexd_socket"
#define MAX_COMMAND_LENGTH 100
#define BUF_SIZE 256
#define SYS_CGROUP_PATH "/sys/fs/cgroup"

#define ERROR_CANNOT_USE_R_S_TOGETHER "Error: Cannot use -r and -s options together.\n"
#define ERROR_INVALID_COMMAND "Error: Invalid command.\n"
#define ERROR_MISSING_CG_GROUP "Error: Missing cg_group argument.\n"
#define ERROR_MISSING_CG_ATTR_FOR_S "Error: Missing cg_attr argument for -s option.\n"
#define ERROR_NO_COMMAND "Error: No command specified.\n"
#define ERROR_RETRIEVE_CGROUP_ATTR_LIST "Error: Failed to retrieve cgroup attributes list.\n"

// Function to remove the socket file if it exists
void rm_socket() {
    if (access(DAEMON_SOCKET_PATH, F_OK) != -1) {
        if (unlink(DAEMON_SOCKET_PATH) == -1) {
            perror("unlink");
            exit(EXIT_FAILURE);
        }
    }
}

// Function to parse the command received from the client
void parse_cmd(const char *cmd, const char **cg_group, const char **cg_opt,
                   const char **cg_attr, const char **cg_type,
                   int *r_flag, int *s_flag, int *t_flag) {
    char *token = strtok((char *)cmd, " ");
    while (token != NULL) {
        if (strcmp(token, "-g") == 0) {
            *cg_group = strtok(NULL, " ");
        } else if (strcmp(token, "-r") == 0) {
            if (*s_flag || *t_flag) {
                printf(ERROR_CANNOT_USE_R_S_TOGETHER);
                exit(EXIT_FAILURE);
            }
            *cg_opt = strtok(NULL, " ");
            *r_flag = 1;
        } else if (strcmp(token, "-s") == 0) {
            if (*r_flag || *t_flag) {
                printf(ERROR_CANNOT_USE_R_S_TOGETHER);
                exit(EXIT_FAILURE);
            }
            *cg_attr = strtok(NULL, " ");
            *s_flag = 1;
        } else if (strcmp(token, "-t") == 0) {
            if (*r_flag || *s_flag) {
                printf(ERROR_CANNOT_USE_R_S_TOGETHER);
                exit(EXIT_FAILURE);
            }
            *cg_type = strtok(NULL, " ");
            *t_flag = 1;
        } else {
            printf(ERROR_INVALID_COMMAND);
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, " ");
    }
}

void ps_client_cmd(int client_fd) {
    // Receive command from client
    char cmd[MAX_COMMAND_LENGTH];
    ssize_t bytes_received = recv(client_fd, cmd, sizeof(cmd) - 1, 0);
    if (bytes_received < 0) {
        perror("recv");
        close(client_fd);
        return;
    }

    cmd[bytes_received] = '\0';
    printf("Received command: %s\n", cmd);

    // Parse command
    const char *cg_group = NULL;
    const char *cg_opt = NULL;
    const char *cg_attr = NULL;
    const char *cg_type = NULL;
    int r_flag = 0, s_flag = 0, t_flag = 0;

    parse_cmd(cmd, &cg_group, &cg_opt, &cg_attr, &cg_type, &r_flag, &s_flag, &t_flag);

    // Validate command
    if (cg_group == NULL) {
        printf(ERROR_MISSING_CG_GROUP);
        close(client_fd);
        return;
    }

    // Construct the full path of the cgroup
    char cg_path[BUF_SIZE];
    snprintf(cg_path, sizeof(cg_path), "%s/%s", SYS_CGROUP_PATH, cg_group);

    if (s_flag && (cg_attr == NULL)) {
        printf(ERROR_MISSING_CG_ATTR_FOR_S);
        close(client_fd);
        return;
    }

    // Execute command based on flags
    int count;
    char **cg_attr_list;

    if (r_flag) {
        if (strcmp(cg_opt, "all") == 0) {
            cg_attr_list = get_cg_list(cg_path, &count);
            if (cg_attr_list == NULL) {
                printf(ERROR_RETRIEVE_CGROUP_ATTR_LIST);
                close(client_fd);
                return;
            }

            for (int i = 0; i < count; i++) {
                show_cg_attr(cg_path, cg_attr_list[i], cg_type);
            }

            free_cg_list(cg_attr_list, count);
        } else {
            show_cg_attr(cg_path, cg_opt, cg_type);
        }
    } else if (s_flag) {
        set_cg_attr(cg_path, cg_attr, cg_type);
    } else if (t_flag) {
        DIR *dir = opendir(cg_path);
        if (dir == NULL) {
            perror("opendir");
            close(client_fd);
            return;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, cg_type, strlen(cg_type)) == 0) {
                show_cg_attr(cg_path, entry->d_name, cg_type);
            }
        }
        closedir(dir);
    } else {
        printf(ERROR_NO_COMMAND);
        close(client_fd);
        return;
    }
    
    close(client_fd);
}

// Main function of the daemon
int main() {
    // Remove socket file if it exists
    rm_socket();

    // Create Unix domain socket for communication
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR option
    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, DAEMON_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Main daemon loop
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        // Fork a child process to handle client command
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            close(client_fd);
            continue;
        } else if (pid == 0) {

            // Child process
            close(server_fd);
            ps_client_cmd(client_fd);
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            close(client_fd);
        }
    }

    close(server_fd);
    rm_socket();
    return EXIT_SUCCESS;
}
