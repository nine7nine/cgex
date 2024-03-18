// cgex_lib.c
#include "libcgex.h"

// Function to read the contents of a file
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
    if (buffer[characters_read - 1] == '\n') {
        buffer[characters_read - 1] = '\0';
    }

    return buffer;
}

// Function to compare strings for sorting
int compare_str(const void *a, const void *b) {
    const char **str1 = (const char **)a;
    const char **str2 = (const char **)b;
    return strcmp(*str1, *str2);
}

// Function to read and print a specific setting in a CGroup
void show_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_type) {
    if (cg_path == NULL || cg_attr == NULL) {
        fprintf(stderr, "Error: NULL input parameter(s) detected\n");
        return;
    }

    // Construct the file path corresponding to the setting
    char file_path[BUF_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", cg_path, cg_attr);

    // Open the file for reading
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    // Print the setting name
    printf("%s: ", cg_attr);

    // If the setting is cgroup.threads, process the PIDs
    if (strcmp(cg_attr, "cgroup.threads") == 0) {
        char *line = NULL;
        size_t len = 0;
        ssize_t read;
        while ((read = getline(&line, &len, file)) != -1) {
            // Remove trailing newline character
            if (line[read - 1] == '\n') {
                line[read - 1] = '\0';
            }

            // Convert PID string to integer
            int pid = atoi(line);

            // Construct the file path for the stat file of the PID
            char pid_stat_path[BUF_SIZE];
            snprintf(pid_stat_path, sizeof(pid_stat_path), "/proc/%d/stat", pid);

            // Read the contents of the stat file
            FILE *stat_file = fopen(pid_stat_path, "r");
            if (stat_file != NULL) {
                // Extract process name from stat file
                char process_name[BUF_SIZE];
                if (fscanf(stat_file, "%*d (%[^)])", process_name) == 1) {
                    // Print PID and process name
                    printf("\n%d - %s", pid, process_name);
                }
                fclose(stat_file);
            }
        }

        if (line) {
            free(line);
        }
    } else {
        // Read the content of the setting file
        char *content = read_cg_attr(file_path);
        if (content != NULL) {
            printf("%s", content);
            free(content);
        }
    }

    printf("\n");
    fclose(file);
}

// Function to set a value for a setting in a CGroup
void set_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_value) {
    if (cg_path == NULL || cg_attr == NULL || cg_value == NULL) {
        fprintf(stderr, "Error: NULL input parameter(s) detected\n");
        return;
    }

    char file_path[BUF_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", cg_path, cg_attr);

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    fprintf(file, "%s\n", cg_value);
    fclose(file);

    printf("Updated %s: %s\n", cg_attr, cg_value);
}

// Function to get a list of settings in a CGroup
char **get_cg_list(const char *cg_path, int *count) {
    DIR *dir = opendir(cg_path);
    if (dir == NULL) {
        perror("opendir");
        return NULL;
    }

    char **cg_attr_list = (char **)malloc(BUF_SIZE * sizeof(char *));
    if (cg_attr_list == NULL) {
        perror("malloc");
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

    qsort(cg_attr_list, *count, sizeof(char *), compare_str);

    return cg_attr_list;
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

