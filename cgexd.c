#include "libcgex.h"

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

    if (!(*r_flag || *s_flag || *t_flag)) {
        fprintf(stderr, ERROR_NO_COMMAND);
        exit(EXIT_FAILURE);
    }
}

void ps_client_cmd(int client_fd, const char *cmd, char *output_buffer, size_t buffer_size) {
    // Parse command
    const char *cg_group = NULL;
    const char *cg_opt = NULL;
    const char *cg_attr = NULL;
    const char *cg_type = NULL;
    int r_flag = 0, s_flag = 0, t_flag = 0;

    parse_cmd(cmd, &cg_group, &cg_opt, &cg_attr, &cg_type, &r_flag, &s_flag, &t_flag);

    // Validate command
    if (cg_group == NULL) {
        snprintf(output_buffer, buffer_size, "%s", ERROR_MISSING_CG_GROUP);
        sockt_send(client_fd, output_buffer, strlen(output_buffer));
        sockt_close(client_fd);
        return;
    }

    // Construct the full path of the cgroup
    char cg_path[BUF_SIZE];
    int path_len = snprintf(cg_path, sizeof(cg_path), "%s/%s", SYS_CGROUP_PATH, cg_group);
    if (path_len >= (int)sizeof(cg_path)) {
        snprintf(output_buffer, buffer_size, "Error: Path buffer overflow.\n");
        sockt_send(client_fd, output_buffer, strlen(output_buffer));
        sockt_close(client_fd);
        return;
    }

    if (s_flag && (cg_attr == NULL)) {
        snprintf(output_buffer, buffer_size, "%s", ERROR_MISSING_CG_ATTR_FOR_S);
        sockt_send(client_fd, output_buffer, strlen(output_buffer));
        sockt_close(client_fd);
        return;
    }

    // Execute command based on flags
    int count;
    char **cg_attr_list;

    if (r_flag) {
        if (strcmp(cg_opt, "all") == 0) {
            cg_attr_list = get_cg_list(cg_path, &count);
            if (cg_attr_list == NULL) {
                snprintf(output_buffer, buffer_size, "%s", ERROR_RETRIEVE_CGROUP_ATTR_LIST);
                sockt_send(client_fd, output_buffer, strlen(output_buffer));
                sockt_close(client_fd);
                return;
            }

            for (int i = 0; i < count; i++) {
                show_cg_attr(cg_path, cg_attr_list[i], cg_type, output_buffer, buffer_size);
                sockt_send(client_fd, output_buffer, strlen(output_buffer));
            }

            free_cg_list(cg_attr_list, count);
        } else {
            show_cg_attr(cg_path, cg_opt, cg_type, output_buffer, buffer_size);
            sockt_send(client_fd, output_buffer, strlen(output_buffer));
        }
    } else if (s_flag) {
        set_cg_attr(cg_path, cg_attr, cg_type, output_buffer, sizeof(output_buffer));
        snprintf(output_buffer, buffer_size, "Updated %s: %s\n", cg_attr, cg_type);
        sockt_send(client_fd, output_buffer, strlen(output_buffer));
    } else if (t_flag) {
        DIR *dir = opendir(cg_path);
        if (dir == NULL) {
            snprintf(output_buffer, buffer_size, "opendir.\n");
            sockt_send(client_fd, output_buffer, strlen(output_buffer));
            sockt_close(client_fd);
            return;
        }

        struct dirent *entry;
        size_t type_len = strlen(cg_type);
        while ((entry = readdir(dir)) != NULL) {
            // Filter cgroup attributes by type
            if (strncmp(entry->d_name, cg_type, type_len) == 0 && entry->d_name[type_len] == '.') {
                show_cg_attr(cg_path, entry->d_name, cg_type, output_buffer, buffer_size);
                sockt_send(client_fd, output_buffer, strlen(output_buffer));
            }
        }
        closedir(dir);
    } else {
        snprintf(output_buffer, buffer_size, "%s", ERROR_NO_COMMAND);
        sockt_send(client_fd, output_buffer, strlen(output_buffer));
    }
    
    sockt_close(client_fd);
}

// Main function of the daemon
int main() {
    sockt_rm();

    // Create Unix domain socket for communication
    int server_fd = sockt_srv_create(DAEMON_SOCKET_PATH);

    char output_buffer[BUF_SIZE];

    // Main daemon loop
    while (1) {
        int client_fd = sockt_cli_accept(server_fd);
        if (client_fd == -1) {
            continue;
        }

        // Fork a child process to handle client command
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "Error: Failed to fork.\n");
            sockt_close(client_fd);
            continue;
        } else if (pid == 0) {
            // Child process
            sockt_close(server_fd);
            char cmd[MAX_COMMAND_LENGTH];
            ssize_t bytes_received = sockt_recv(client_fd, cmd, sizeof(cmd));
            if (bytes_received < 0) {
                sockt_close(client_fd);
                return EXIT_FAILURE;
            }
            //printf("Received command: %s\n", cmd);
            ps_client_cmd(client_fd, cmd, output_buffer, sizeof(output_buffer));
            exit(EXIT_SUCCESS);
        } else {
            // Parent process
            sockt_close(client_fd);
        }
    }

    sockt_close(server_fd);
    sockt_rm();
    return EXIT_SUCCESS;
}
