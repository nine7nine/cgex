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

    // Read and print the setting
    char *content = read_file_contents(file_path);
    if (content != NULL) {
        // Print the setting name
        printf("%s: %s", setting, content);
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
            settings_list[(*count)++] = strdup(entry->d_name); // Allocate memory for each setting name
        }
    }
    closedir(dir);

    return settings_list;
}

// Function to free memory allocated by settings list
void free_settings_list(char **settings_list, int count) {
    if (settings_list == NULL) {
        return; // Check for NULL pointer to avoid dereferencing
    }
    for (int i = 0; i < count; i++) {
        if (settings_list[i] != NULL) { // Check for NULL pointer to avoid dereferencing
            free(settings_list[i]); // Free memory allocated for each setting name
        }
    }
    free(settings_list); // Free memory allocated for the settings list array itself
}
