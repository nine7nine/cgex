#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "libcgex.h"

#define ERROR_USAGE "Usage: %s -g <cgroup_path> [-r <all|setting> | -s <setting> <value> | -t <type>]\n"
#define ERROR_RS_TOGETHER "Error: Cannot use -r and -s options together.\n"
#define ERROR_MISSING_ARG "Error: Missing setting or value argument for -s option.\n"
#define BUF_SIZE 256

int main(int argc, char *argv[]) {
    const char *cgroup_path = NULL;
    const char *operation = NULL;
    const char *setting = NULL;
    const char *type = NULL;

    int opt, r_flag = 0, s_flag = 0, t_flag = 0;

    while ((opt = getopt(argc, argv, "g:r:s:t:")) != -1) {
        switch (opt) {
            case 'g':
                cgroup_path = optarg;
                break;
            case 'r':
                if (s_flag || t_flag) {
                    fprintf(stderr, ERROR_RS_TOGETHER, argv[0]);
                    return EXIT_FAILURE;
                }
                operation = optarg;
                r_flag = 1;
                break;
            case 's':
                if (r_flag || t_flag) {
                    fprintf(stderr, ERROR_RS_TOGETHER, argv[0]);
                    return EXIT_FAILURE;
                }
                setting = optarg;
                s_flag = 1;
                break;
            case 't':
                if (r_flag || s_flag) {
                    fprintf(stderr, ERROR_RS_TOGETHER, argv[0]);
                    return EXIT_FAILURE;
                }
                type = optarg;
                t_flag = 1;
                break;
            default:
                fprintf(stderr, ERROR_USAGE, argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (cgroup_path == NULL) {
        fprintf(stderr, ERROR_USAGE, argv[0]);
        return EXIT_FAILURE;
    }

    if (s_flag && (!setting || optind >= argc)) {
        fprintf(stderr, ERROR_MISSING_ARG);
        return EXIT_FAILURE;
    }

    int count;
    char **settings_list;

    if (r_flag) {
        if (strcmp(operation, "all") == 0) {
            settings_list = get_settings_list(cgroup_path, &count);
            if (settings_list == NULL) {
                return EXIT_FAILURE;
            }

            for (int i = 0; i < count; i++) {
                read_and_print_setting(cgroup_path, settings_list[i], type);
            }

            free_settings_list(settings_list, count);
        } else {
            read_and_print_setting(cgroup_path, operation, type);
        }
    } else if (s_flag) {
        set_cgroup_setting(cgroup_path, setting, argv[optind]);
    } else if (t_flag) {
        DIR *dir = opendir(cgroup_path);
        if (dir == NULL) {
            perror("opendir");
            return EXIT_FAILURE;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, type, strlen(type)) == 0) {
                read_and_print_setting(cgroup_path, entry->d_name, type);
            }
        }
        closedir(dir);
    } else {
        fprintf(stderr, "Error: No operation specified. Use either -r or -s option.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
