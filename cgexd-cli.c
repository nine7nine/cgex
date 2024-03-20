#include "libcgex.h"

// Helper function to concatenate strings safely
void concat_str(char *dest, const char *src) {
    strncat(dest, src, MAX_COMMAND_LENGTH - strlen(dest) - 1);
}

int main(int argc, char *argv[]) {
    const char *cg_group = NULL;
    const char *cg_opt = NULL;
    const char *cg_attr = NULL;
    const char *cg_type = NULL;

    int opt, r_flag = 0, s_flag = 0, t_flag = 0;

    while ((opt = getopt(argc, argv, "g:r:s:t:")) != -1) {
        switch (opt) {
            case 'g':
                cg_group = optarg;
                break;
            case 'r':
                if (s_flag || t_flag) {
                    fprintf(stderr, ERROR_RS_TOGETHER);
                    return EXIT_FAILURE;
                }
                cg_opt = optarg;
                r_flag = 1;
                break;
            case 's':
                if (r_flag || t_flag) {
                    fprintf(stderr, ERROR_RS_TOGETHER);
                    return EXIT_FAILURE;
                }
                cg_attr = optarg;
                s_flag = 1;
                break;
            case 't':
                if (r_flag || s_flag) {
                    fprintf(stderr, ERROR_RS_TOGETHER);
                    return EXIT_FAILURE;
                }
                cg_type = optarg;
                t_flag = 1;
                break;
            default:
                fprintf(stderr, ERROR_USAGE, argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (cg_group == NULL) {
        fprintf(stderr, ERROR_MISSING_GROUP);
        return EXIT_FAILURE;
    }

    // Construct command
    char cmd[MAX_COMMAND_LENGTH] = "-g ";
    concat_str(cmd, cg_group);
    if (r_flag) {
        concat_str(cmd, " -r");
        if (strcmp(cg_opt, "all") == 0) {
            concat_str(cmd, " all");
        } else {
            concat_str(cmd, " ");
            concat_str(cmd, cg_opt);
        }
    } else if (s_flag) {
        if (optind >= argc) {
            fprintf(stderr, ERROR_MISSING_ARG);
            return EXIT_FAILURE;
        }
        concat_str(cmd, " -s ");
        concat_str(cmd, cg_attr);
        concat_str(cmd, " ");
        concat_str(cmd, argv[optind]);
    } else if (t_flag) {
        concat_str(cmd, " -t ");
        concat_str(cmd, cg_type);
    }

    // Connect to the daemon
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "socket.\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, DAEMON_SOCKET_PATH, sizeof(server_addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "connect.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Send command to daemon
    if (send(sockfd, cmd, strlen(cmd), 0) < 0) {
        fprintf(stderr, "send.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    //printf("Sent command to daemon: %s\n", cmd);

    // Receive and print response from daemon
    char response[BUF_SIZE];
    ssize_t bytes_received;
    while ((bytes_received = recv(sockfd, response, sizeof(response) - 1, 0)) > 0) {
        response[bytes_received] = '\0';
        printf("%s", response);
    }

    if (bytes_received < 0) {
        fprintf(stderr, "recv.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    close(sockfd);

    return EXIT_SUCCESS;
}
