#ifndef UTILS_H
#define UTILS_H

#ifdef _WIN32
#define PATH_SEP "\\"
#include <direct.h>
#include <windows.h>
#include <Lmcons.h>
#define GetCWD _getcwd
#else
#define PATH_SEP "/"
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#define GetCurrentDir getcwd
#endif

char *recursive_file_search(char fullpath[], char filename[]);

char *find_file(char filename[]);

char *get_var(const char *var);

void tokenize_cmd(char cmd[], char tokens[][50], int *no_of_tokens);

void set_pwd();

void set_home_directory();

void set_hostname();

void set_username();

void init_prompt();

#endif