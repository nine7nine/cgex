// cgex_lib.h
#ifndef LIBCGEX_H
#define LIBCGEX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

#define BUF_SIZE 256

// Function to read the contents of a file
char *read_file_contents(const char *file_path);

// Function to read and print a specific setting in a CGroup
void read_and_print_setting(const char *cgroup_path, const char *setting, const char *type);

// Function to set a value for a setting in a CGroup
void set_cgroup_setting(const char *cgroup_path, const char *setting, const char *value);

// Function to get a list of settings in a CGroup
char **get_settings_list(const char *cgroup_path, int *count);

// Function to free memory allocated by settings list
void free_settings_list(char **settings_list, int count);

#endif

