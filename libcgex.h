// libcgex.h
#ifndef LIBCGEX_H
#define LIBCGEX_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <syslog.h>
#include <dirent.h>

#define BUF_SIZE 256
#define MAX_COMMAND_LENGTH 100
#define DAEMON_SOCKET_PATH "/var/run/cgexd_socket"
#define SYS_CGROUP_PATH "/sys/fs/cgroup"

// Error Messages
#define ERROR_INVALID_COMMAND "Error: Invalid command.\n"
#define ERROR_MISSING_ARG "Error: Missing cg_attr or cg_value argument for -s option.\n"
#define ERROR_MISSING_GROUP "Error: Missing mandatory argument -g\n"
#define ERROR_MISSING_CG_ATTR_FOR_S "Error: Missing cg_attr argument for -s option.\n"
#define ERROR_MISSING_CG_GROUP "Error: Missing cg_group argument.\n"
#define ERROR_NO_COMMAND "Error: No command specified.\n"
#define ERROR_RETRIEVE_CGROUP_ATTR_LIST "Error: Failed to retrieve cgroup attributes list.\n"
#define ERROR_RS_TOGETHER "Error: Cannot use -r and -s options together.\n"
#define ERROR_USAGE "Usage: %s -g <cg_group> [-r <all|cg_attr> | -s <cg_attr> <cg_value> | -t <cg_type>]\n"

// Function to read the contents of a file
char *read_cg_attr(const char *file_path);

// Function to read and print a specific setting in a CGroup
void show_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_type);

// Function to set a value for a setting in a CGroup
void set_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_value);

// Function to get a list of settings in a CGroup
char **get_cg_list(const char *cg_path, int *count);

// Function to free memory allocated by settings list
void free_cg_list(char **cg_attr_list, int count);

// Helper function to print process information
void ps_stat_info(int pid);

#endif

