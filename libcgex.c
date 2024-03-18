// cgex_lib.c
#include "libcgex.h"

// Function to read the contents of a file
char *read_file_contents(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    static char buffer[BUF_SIZE];
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return NULL;
    }

    fclose(file);
    return buffer;
}

// Function to read and print a specific setting in a CGroup
void read_and_print_setting(const char *cgroup_path, const char *setting, const char *type) {
    // Construct the file path corresponding to the setting
    char file_path[BUF_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", cgroup_path, setting);

    // Check if the setting matches the specified type prefix
    if (type != NULL && strncmp(setting, type, strlen(type)) != 0) {
        return; // Skip this setting if it doesn't match the specified type prefix
    }

    // If the setting is cgroup.threads, process the PIDs
    if (strcmp(setting, "cgroup.threads") == 0) {
        // Read the content of the setting file
        FILE *file = fopen(file_path, "r");
        if (file == NULL) {
            perror("fopen");
            return;
        }

        printf("%s:\n", setting); // Print the setting name
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
                    printf("%d - %s\n", pid, process_name);
                }
                fclose(stat_file);
            }
        }

        fclose(file);
        if (line) {
            free(line);
        }
    } else {
        // For settings other than cgroup.threads, print the content as usual
        char *content = read_file_contents(file_path);
        if (content != NULL) {
            printf("%s: %s", setting, content); // Print setting name and content without newline
        }
    }
}

// Function to set a value for a setting in a CGroup
void set_cgroup_setting(const char *cgroup_path, const char *setting, const char *value) {
    char file_path[BUF_SIZE];
    snprintf(file_path, sizeof(file_path), "%s/%s", cgroup_path, setting);

    FILE *file = fopen(file_path, "w");
    if (file == NULL) {
        perror("fopen");
        return;
    }

    fprintf(file, "%s\n", value);
    fclose(file);

    printf("Updated %s: %s\n", setting, value);
}

// Function to get a list of settings in a CGroup
char **get_settings_list(const char *cgroup_path, int *count) {
    DIR *dir = opendir(cgroup_path);
    if (dir == NULL) {
        perror("opendir");
        return NULL;
    }

    char **settings_list = (char **)malloc(BUF_SIZE * sizeof(char *));
    if (settings_list == NULL) {
        perror("malloc");
        closedir(dir);
        return NULL;
    }

    *count = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            settings_list[(*count)++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    return settings_list;
}

// Function to free memory allocated by settings list
void free_settings_list(char **settings_list, int count) {
    if (settings_list == NULL) {
        return;
    }
    for (int i = 0; i < count; i++) {
        if (settings_list[i] != NULL) {
            free(settings_list[i]);
        }
    }
    free(settings_list);
}
