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

/* libcgex.c:
 * 
 * Core Libcgex.c stuff
 *
 */ 

#define BUF_SIZE 512
#define MAX_COMMAND_LENGTH 100
#define DAEMON_SOCKET_PATH "/var/run/cgexd_socket"
#define SYS_CGROUP_PATH "/sys/fs/cgroup"

// Error Messages
#define ERROR_INVALID_COMMAND "Error: Invalid command.\n"
#define ERROR_MISSING_ARG "Error: Missing cg_attr or cg_value argument for -s option.\n"
#define ERROR_MISSING_GROUP "Error: Missing mandatory argument -g\n"
#define ERROR_MISSING_CG_TYPE "Error: Missing cg_type data"
#define ERROR_MISSING_CG_ATTR_FOR_S "Error: Missing cg_attr argument for -s option.\n"
#define ERROR_MISSING_CG_GROUP "Error: Missing cg_group argument.\n"
#define ERROR_NO_COMMAND "Error: No command specified.\n"
#define ERROR_RETRIEVE_CGROUP_ATTR_LIST "Error: Failed to retrieve cgroup attributes list.\n"
#define ERROR_RS_TOGETHER "Error: Cannot use -r and -s options together.\n"
#define ERROR_USAGE "Usage: %s -g <cg_group> [-r <all|cg_attr> | -s <cg_attr> <cg_value> | -t <cg_type>]\n"

// Function to read the contents of a file
char *read_cg_attr(const char *file_path);
// Function to read and print a specific setting in a CGroup
void show_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_type, char *output_buffer, size_t buffer_size);
// Function to set a value for a setting in a CGroup
void set_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_value, char *output_buffer, size_t buffer_size);
// Function to get a list of settings in a CGroup
char **get_cg_list(const char *cg_path, int *count);
// Function to free memory allocated by settings list
void free_cg_list(char **cg_attr_list, int count);
// Helper function to print process information
void ps_stat_info(int pid, char *output_buffer, size_t buffer_size);

/* libcgex-utils:
 * 
 * Keeping some of the more simple/generic abstractions outside of libcgex.c
 *
 */ 

// Function to compare strings for sorting
int comp_str(const void *a, const void *b);
// Helper function to concatenate strings safely
void concat_str(char **dest, const char *src);
// Function to construct file paths
void cg_fs_path(char *file_path, size_t size, const char *cg_path, const char *cg_attr);
// Function to handle errors and exit in cgexd.c
void cgexd_err(const char *message);
// Function to handle errors in libcgex.c
void libcgex_err(const char *message, char *output_buffer, size_t buffer_size);
// Function to handle string manipulation
void trail_rm(char *str);
// Function to free dynamically allocated memory and return EXIT_FAILURE
void clr_exit(char *cmd, char *response, int sockfd);
// Function to create a Unix domain socket
int sockt_srv_create(const char *socket_path);
// Function to establish a connection to the daemon
int sockt_srv_connect(const char *socket_path);
// Function to accept a client connection
int sockt_cli_accept(int server_fd);
// Function to send data to a client
void sockt_send(int client_fd, const char *data, size_t data_size);
// Function to receive data from a client
ssize_t sockt_recv(int client_fd, char *buffer, size_t buffer_size);
// Function to close a socket
void sockt_close(int fd);
// Function to remove the socket file if it exists
void sockt_rm();

#endif
