// libcgex.c
#include "libcgex.h"

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

    char *buffer = NULL;
    size_t bufsize = 0;
    ssize_t characters_read = getline(&buffer, &bufsize, file);
    fclose(file);

    if (characters_read == -1 || (characters_read == 1 && buffer[0] == '\n')) {
        free(buffer);
        return NULL;
    }

    rm_trail(buffer);

    return buffer;
}

// Helper function to print process information
void ps_stat_info(int pid, char *output_buffer, size_t buffer_size) {
    char pid_stat_path[BUF_SIZE];
    snprintf(pid_stat_path, sizeof(pid_stat_path), "/proc/%d/stat", pid);

    FILE *stat_file = fopen(pid_stat_path, "r");
    if (stat_file != NULL) {
        char process_name[BUF_SIZE];
        if (fscanf(stat_file, "%*d (%[^)])", process_name) == 1) {
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

    char file_path[BUF_SIZE];
    cg_fs_path(file_path, sizeof(file_path), cg_path, cg_attr);

    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        snprintf(output_buffer, buffer_size, "Failed to open file: %s\n", file_path);
        return;
    }

    if (cg_type != NULL) {
        size_t type_len = strlen(cg_type);
        if (strncmp(cg_attr, cg_type, type_len) != 0 || cg_attr[type_len] != '.') {
            fclose(file);
            return;
        }
    }

    int len = snprintf(output_buffer, buffer_size, "%s: ", cg_attr);

    if (strcmp(cg_attr, "cgroup.threads") == 0) {
        snprintf(output_buffer + len, buffer_size - len, "\n");
        len++;
        
        char line[BUF_SIZE];
        while (fgets(line, sizeof(line), file) != NULL) {
            rm_trail(line);

            int pid = atoi(line);

            ps_stat_info(pid, output_buffer + len, buffer_size - len);
            len = strlen(output_buffer);
            snprintf(output_buffer + len, buffer_size - len, "\n");
        }
    } else {
        char *content = read_cg_attr(file_path);
        if (content != NULL) {
            len += snprintf(output_buffer + len, buffer_size - len, "%s", content);
            free(content);
        }
    }

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

    snprintf(output_buffer, buffer_size, "Updated %s: %s\n", cg_attr, cg_value);

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Failed to open file: %s\n", file_path);
        return;
    }

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
