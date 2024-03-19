#include "libcgex.h"

#define ERROR_USAGE "Usage: %s -g <cg_group> [-r <all|cg_attr> | -s <cg_attr> <cg_value> | -t <cg_type>]\n"
#define ERROR_RS_TOGETHER "Error: Cannot use -r and -s options together.\n"
#define ERROR_MISSING_ARG "Error: Missing cg_attr or cg_value argument for -s option.\n"
#define BUF_SIZE 256
#define SYS_CGROUP_PATH "/sys/fs/cgroup"

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
        fprintf(stderr, ERROR_USAGE, argv[0]);
        return EXIT_FAILURE;
    }

    // Construct the full path of the cgroup
    char cg_path[BUF_SIZE];
    snprintf(cg_path, sizeof(cg_path), "%s/%s", SYS_CGROUP_PATH, cg_group);

    if (s_flag && (!cg_attr || optind >= argc)) {
        fprintf(stderr, ERROR_MISSING_ARG);
        return EXIT_FAILURE;
    }

    int count;
    char **cg_attr_list;

    if (r_flag) {
        if (strcmp(cg_opt, "all") == 0) {
            cg_attr_list = get_cg_list(cg_path, &count);
            if (cg_attr_list == NULL) {
                return EXIT_FAILURE;
            }

            for (int i = 0; i < count; i++) {
                show_cg_attr(cg_path, cg_attr_list[i], cg_type);
            }

            free_cg_list(cg_attr_list, count);
        } else {
            show_cg_attr(cg_path, cg_opt, cg_type);
        }
    } else if (s_flag) {
        set_cg_attr(cg_path, cg_attr, argv[optind]);
    } else if (t_flag) {
        DIR *dir = opendir(cg_path);
        if (dir == NULL) {
            perror("opendir");
            return EXIT_FAILURE;
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, cg_type, strlen(cg_type)) == 0) {
                show_cg_attr(cg_path, entry->d_name, cg_type);
            }
        }
        closedir(dir);
    } else {
        fprintf(stderr, "Error: No cg_opt specified. Use either -r or -s option.\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
