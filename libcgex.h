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
char *read_cg_attr(const char *file_path);

// Function to read and print a specific setting in a CGroup
void show_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_type);

// Function to set a value for a setting in a CGroup
void set_cg_attr(const char *cg_path, const char *cg_attr, const char *cg_value);

// Function to get a list of settings in a CGroup
char **get_cg_list(const char *cg_path, int *count);

// Function to free memory allocated by settings list
void free_cg_list(char **cg_attr_list, int count);

#endif

