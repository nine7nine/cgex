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

// Function to handle errors
void libcgex_err(const char *message, char *output_buffer, size_t buffer_size) {
    snprintf(output_buffer, buffer_size, "%s\n", message);
}

// Function to handle string manipulation
void rm_trail(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Function to free memory allocated by settings list
void free_cg_list(char **cg_attr_list, int count) {
    if (cg_attr_list == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        if (cg_attr_list[i] != NULL) {
            free(cg_attr_list[i]);
        }
    }
    free(cg_attr_list);
}

// Helper function to read file contents
char *read_cg_attr(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    // Dynamically allocate memory for file contents
    char *buffer = NULL;
    size_t bufsize = 0;
    ssize_t characters_read = getline(&buffer, &bufsize, file);
    fclose(file);

    // Check if getline encountered an error or if the value is empty
    if (characters_read == -1 || (characters_read == 1 && buffer[0] == '\n')) {
        free(buffer);
        return NULL;
    }

    // Remove trailing newline character
    rm_trail(buffer);

    return buffer;
}

// Helper function to print process information
void ps_stat_info(int pid, char *output_buffer, size_t buffer_size) {
    // Construct the file path for the stat file of the PID
    char pid_stat_path[BUF_SIZE];
    snprintf(pid_stat_path, sizeof(pid_stat_path), "/proc/%d/stat", pid);

    // Read the contents of the stat file
    FILE *stat_file = fopen(pid_stat_path, "r");
    if (stat_file != NULL) {
        // Extract process name from stat file
        char process_name[BUF_SIZE];
        if (fscanf(stat_file, "%*d (%[^)])", process_name) == 1) {
            // Append PID and process name to the output buffer
            snprintf(output_buffer + strlen(output_buffer), buffer_size - strlen(output_buffer), "%d - %s", pid, process_name);
        }
        fclose(stat_file);
    }
}

// Function to read and print a specific setting in a CGroup
void show_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_type, char *output_buffer, size_t buffer_size) {
    if (cg_path == NULL || cg_attr == NULL) {
        libcgex_err("NULL input parameter(s) detected", output_buffer, buffer_size);
        return;
    }

    // Construct the file path corresponding to the setting
    char file_path[BUF_SIZE];
    cg_fs_path(file_path, sizeof(file_path), cg_path, cg_attr);

    // Open the file for reading
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        snprintf(output_buffer, buffer_size, "Failed to open file: %s\n", file_path);
        return;
    }

    // cg_type (cgroup.controllers)) are the prefix word ending with a period.
    if (cg_type != NULL) {
        size_t type_len = strlen(cg_type);
        if (strncmp(cg_attr, cg_type, type_len) != 0 || cg_attr[type_len] != '.') {
            fclose(file);
            return;
        }
    }

    // Write the setting name to the buffer
    int len = snprintf(output_buffer, buffer_size, "%s: ", cg_attr);

    // If the setting is cgroup.threads, process the PIDs
    if (strcmp(cg_attr, "cgroup.threads") == 0) {
        // Print newline before printing PIDs
        snprintf(output_buffer + len, buffer_size - len, "\n");
        len++;
        
        char line[BUF_SIZE];
        while (fgets(line, sizeof(line), file) != NULL) {
            // Remove trailing newline character
            rm_trail(line);

            int pid = atoi(line);

            // Print process information
            ps_stat_info(pid, output_buffer + len, buffer_size - len);
            len = strlen(output_buffer);
            snprintf(output_buffer + len, buffer_size - len, "\n");
        }
    } else {
        // Read the content of the setting file into the buffer
        char *content = read_cg_attr(file_path);
        if (content != NULL) {
            len += snprintf(output_buffer + len, buffer_size - len, "%s", content);
            free(content);
        }
    }

    // Append newline character
    snprintf(output_buffer + len, buffer_size - len, "\n");

    fclose(file);
}

// Function to set a value for a setting in a CGroup
void set_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_value, char *output_buffer, size_t buffer_size) {
    if (cg_path == NULL || cg_attr == NULL || cg_value == NULL) {
        libcgex_err("NULL input parameter(s) detected", output_buffer, buffer_size);
        return;
    }

    char file_path[BUF_SIZE];
    cg_fs_path(file_path, sizeof(file_path), cg_path, cg_attr);

    // Write the update message to the buffer
    snprintf(output_buffer, buffer_size, "Updated %s: %s\n", cg_attr, cg_value);

    // Open the file for writing
    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", file_path);
        return;
    }

    // Write the value to the file
    fprintf(file, "%s\n", cg_value);
    fclose(file);
}

// Function to get a list of settings in a CGroup
char **get_cg_list(const char *cg_path, int *count) {
    DIR *dir = opendir(cg_path);
    if (dir == NULL) {
        fprintf(stderr, "Failed to open directory: %s\n", cg_path);
        return NULL;
    }

    char **cg_attr_list = (char **)malloc(BUF_SIZE * sizeof(char *));
    if (cg_attr_list == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        closedir(dir);
        return NULL;
    }

    *count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            cg_attr_list[(*count)++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    qsort(cg_attr_list, *count, sizeof(char *), comp_str);

    return cg_attr_list;
}

