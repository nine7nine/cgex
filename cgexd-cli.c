#include "libcgex.h"

int main(int argc, char *argv[]) {
    const char *cg_group = NULL;
    const char *cg_opt = NULL;
    const char *cg_attr = NULL;
    const char *cg_type = NULL;

    int opt, r_flag = 0, s_flag = 0, t_flag = 0;

    // Initialize command buffer
    char *cmd = malloc(MAX_COMMAND_LENGTH);
    if (cmd == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return EXIT_FAILURE;
    }
    cmd[0] = '\0';

    // Initialize response buffer
    char *response = malloc(BUF_SIZE);
    if (response == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(cmd);
        return EXIT_FAILURE;
    }
    response[0] = '\0';

    while ((opt = getopt(argc, argv, "g:r:s:t:")) != -1) {
        switch (opt) {
            case 'g':
                cg_group = optarg;
                break;
            case 'r':
                if (s_flag || t_flag) {
                    fprintf(stderr, "%s", ERROR_RS_TOGETHER);
                    clr_exit(cmd, response, -1);
                }
                cg_opt = optarg;
                r_flag = 1;
                break;
            case 's':
                if (r_flag || t_flag) {
                    fprintf(stderr, "%s", ERROR_RS_TOGETHER);
                    clr_exit(cmd, response, -1);
                }
                cg_attr = optarg;
                s_flag = 1;
                break;
            case 't':
                if (r_flag || s_flag) {
                    fprintf(stderr, "%s", ERROR_RS_TOGETHER);
                    clr_exit(cmd, response, -1);
                }
                cg_type = optarg;
                t_flag = 1;
                break;
            default:
                fprintf(stderr, ERROR_USAGE, argv[0]);
                clr_exit(cmd, response, -1);
        }
    }

    if (cg_group == NULL) {
        fprintf(stderr, "%s", ERROR_MISSING_CG_GROUP);
        clr_exit(cmd, response, -1);
    }

    // Construct command
    concat_str(&cmd, "-g ");
    concat_str(&cmd, cg_group);
    if (r_flag) {
        concat_str(&cmd, " -r");
        if (strcmp(cg_opt, "all") == 0) {
            concat_str(&cmd, " all");
        } else {
            concat_str(&cmd, " ");
            concat_str(&cmd, cg_opt);
        }
    } else if (s_flag) {
        if (optind >= argc) {
            fprintf(stderr, "%s", ERROR_MISSING_ARG);
            clr_exit(cmd, response, -1);
        }
        concat_str(&cmd, " -s ");
        concat_str(&cmd, cg_attr);
        concat_str(&cmd, " ");
        concat_str(&cmd, argv[optind]);
    } else if (t_flag) {
        concat_str(&cmd, " -t ");
        concat_str(&cmd, cg_type);
    }

    // Connect to the daemon
    int sockfd = sockt_srv_connect(DAEMON_SOCKET_PATH);
    if (sockfd < 0) {
        // Handle connection error
        exit(EXIT_FAILURE);
    }

    // Send command to daemon
    printf("Sending command to daemon: %s\n", cmd);
    sockt_send(sockfd, cmd, strlen(cmd));

    // Receive and print response from daemon
    ssize_t bytes_received;
    while ((bytes_received = sockt_recv(sockfd, response, BUF_SIZE - 1)) > 0) {
        response[bytes_received] = '\0';
        printf("%s", response);
    }

    if (bytes_received < 0) {
        fprintf(stderr, "Error: Failed to receive response from daemon\n");
        clr_exit(cmd, response, sockfd);
    }

    free(cmd);
    free(response);
    sockt_close(sockfd);

    return EXIT_SUCCESS;
}
