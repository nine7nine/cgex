// libcgex.c
#include "libcgex.h"

// Function to compare strings for sorting
int comp_str(const void *a, const void *b) {
    const char **str1 = (const char **)a;
    const char **str2 = (const char **)b;
    return strcmp(*str1, *str2);
}

// Function to construct file paths
void cg_fs_path(char *file_path, size_t size, const char *cg_path, const char *cg_attr) {
    snprintf(file_path, size, "%s/%s", cg_path, cg_attr);
}

// Function to handle string manipulation
void trail_rm(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Function to handle errors and exit
void cgexd_err(const char *message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(EXIT_FAILURE);
}

// Function to handle errors
void libcgex_err(const char *message, char *output_buffer, size_t buffer_size) {
    snprintf(output_buffer, buffer_size, "%s\n", message);
}

// Helper function to concatenate strings safely
void concat_str(char **dest, const char *src) {
    size_t current_len = strlen(*dest);
    size_t src_len = strlen(src);
    *dest = realloc(*dest, current_len + src_len + 1);
    if (*dest == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcat(*dest, src);
}

// Function to free dynamically allocated memory and return EXIT_FAILURE
void clr_exit(char *cmd, char *response, int sockfd) {
    free(cmd);
    free(response);
    sockt_close(sockfd);
    exit(EXIT_FAILURE);
}

// Socket Abstractions

// Function to create a Unix domain socket
int sockt_srv_create(const char *socket_path) {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "Error: Failed to create socket\n");
        return -1;
    }

    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
        fprintf(stderr, "Error: Failed to set socket option\n");
        close(server_fd);
        return -1;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        fprintf(stderr, "Error: Failed to bind socket\n");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) == -1) {
        fprintf(stderr, "Error: Failed to listen on socket\n");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

// Function to establish a connection to the daemon
int sockt_srv_connect(const char *socket_path) {
    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "Error: Failed to create socket\n");
        return -1;
    }

    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, socket_path, sizeof(server_addr.sun_path) - 1);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Error: Failed to connect to daemon\n");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

// Function to accept a client connection
int sockt_cli_accept(int server_fd) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == -1) {
        fprintf(stderr, "Error: Failed to accept connection.\n");
    }
    return client_fd;
}

// Function to send data to a client
void sockt_send(int client_fd, const char *data, size_t data_size) {
    size_t total_bytes_sent = 0;
    ssize_t bytes_sent;

    while (total_bytes_sent < data_size) {
        bytes_sent = send(client_fd, data + total_bytes_sent, data_size - total_bytes_sent, 0);
        if (bytes_sent == -1) {
            fprintf(stderr, "Error: Failed to send data.\n");
            break;
        }
        total_bytes_sent += bytes_sent;
    }
}

// Function to receive data from a client
ssize_t sockt_recv(int client_fd, char *buffer, size_t buffer_size) {
    ssize_t bytes_received = recv(client_fd, buffer, buffer_size - 1, 0);
    if (bytes_received < 0) {
        fprintf(stderr, "Error: Failed to receive data.\n");
    } else {
        buffer[bytes_received] = '\0';
    }
    return bytes_received;
}

// Function to close a socket
void sockt_close(int fd) {
    if (close(fd) == -1) {
        fprintf(stderr, "Error: Failed to close socket.\n");
    }
}

// Function to remove the socket file if it exists
void sockt_rm() {
    if (access(DAEMON_SOCKET_PATH, F_OK) != -1) {
        if (unlink(DAEMON_SOCKET_PATH) == -1) {
            fprintf(stderr, "Error: Failed to remove socket file.\n");
            exit(EXIT_FAILURE);
        }
    }
}
