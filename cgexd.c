#include "libcgex.h"

// Function to remove the socket file if it exists
void rm_socket() {
    if (access(DAEMON_SOCKET_PATH, F_OK) != -1) {
        if (unlink(DAEMON_SOCKET_PATH) == -1) {
            fprintf(stderr, "Error: Failed to remove socket file.\n");
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
                fprintf(stderr, ERROR_RS_TOGETHER);
                exit(EXIT_FAILURE);
            }
            *cg_opt = strtok(NULL, " ");
            *r_flag = 1;
        } else if (strcmp(token, "-s") == 0) {
            if (*r_flag || *t_flag) {
                fprintf(stderr, ERROR_RS_TOGETHER);
                exit(EXIT_FAILURE);
            }
            *cg_attr = strtok(NULL, " ");
            // Handle Delineators '--'
            token = strtok(NULL, " ");
            if (token != NULL && strcmp(token, "--") == 0) {
                *s_flag = 1;
                *cg_type = strtok(NULL, " ");
                break;
            } else {
                *s_flag = 1;
                *cg_type = token;
                break;
            }
        } else if (strcmp(token, "-t") == 0) {
            if (*r_flag || *s_flag) {
                fprintf(stderr, ERROR_RS_TOGETHER);
                exit(EXIT_FAILURE);
            }
            *cg_type = strtok(NULL, " ");
            *t_flag = 1;
        } else {
            fprintf(stderr, ERROR_INVALID_COMMAND);
            exit(EXIT_FAILURE);
        }
        token = strtok(NULL, " ");
    }

    // If no command is specified, set the corresponding flag
    if (!(*r_flag || *s_flag || *t_flag)) {
        fprintf(stderr, ERROR_NO_COMMAND);
        exit(EXIT_FAILURE);
    }
}

void ps_client_cmd(int client_fd, const char *cmd) {
    // Parse command
    const char *cg_group = NULL;
    const char *cg_opt = NULL;
    const char *cg_attr = NULL;
    const char *cg_type = NULL;
    int r_flag = 0, s_flag = 0, t_flag = 0;

    parse_cmd(cmd, &cg_group, &cg_opt, &cg_attr, &cg_type, &r_flag, &s_flag, &t_flag);

    // Validate command
    if (cg_group == NULL) {
        fprintf(stderr, ERROR_MISSING_CG_GROUP);
        close(client_fd);
        return;
    }

    // Construct the full path of the cgroup
    char cg_path[BUF_SIZE];
    int path_len = snprintf(cg_path, sizeof(cg_path), "%s/%s", SYS_CGROUP_PATH, cg_group);
    if (path_len >= (int)sizeof(cg_path)) {
        fprintf(stderr, "Error: Path buffer overflow.\n");
        close(client_fd);
        return;
    }

    if (s_flag && (cg_attr == NULL)) {
        fprintf(stderr, ERROR_MISSING_CG_ATTR_FOR_S);
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
                fprintf(stderr, ERROR_RETRIEVE_CGROUP_ATTR_LIST);
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
        fprintf(stderr, ERROR_NO_COMMAND);
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
        fprintf(stderr, "Error: Failed to create socket.\n");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR option
    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        fprintf(stderr, "Error: Failed to set socket option.\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, DAEMON_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "Error: Failed to bind socket.\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        fprintf(stderr, "Error: Failed to listen on socket.\n");
        exit(EXIT_FAILURE);
    }

    // Main daemon loop
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd == -1) {
            fprintf(stderr, "Error: Failed to accept connection.\n");
            continue;
        }

        // Fork a child process to handle client command
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Error: Failed to fork.\n");
            close(client_fd);
            continue;
        } else if (pid == 0) {

            // Child process
            close(server_fd);
            char cmd[MAX_COMMAND_LENGTH];
            ssize_t bytes_received = recv(client_fd, cmd, sizeof(cmd) - 1, 0);
            if (bytes_received < 0) {
                fprintf(stderr, "Error: Failed to receive command.\n");
                close(client_fd);
                return EXIT_FAILURE;
            }
            cmd[bytes_received] = '\0';
            printf("Received command: %s\n", cmd);
            ps_client_cmd(client_fd, cmd);
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
