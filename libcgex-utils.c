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
void rm_trail(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Function to remove the socket file if it exists
void rm_socket() {
    if (access(DAEMON_SOCKET_PATH, F_OK) != -1) {
        if (unlink(DAEMON_SOCKET_PATH) == -1) {
            fprintf(stderr, "Error: Failed to remove socket file.\n");
            exit(EXIT_FAILURE);
        }
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
