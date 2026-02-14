#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <io.h>
#include "utils.h"

#ifdef _WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

#define MAX_PATH_LEN 1024
#define MAX_NO_OF_PATHS 30

/**
 * Builds a full file path and checks if the file is executable.
 * Returns the path if executable, NULL otherwise.
 * Caller must free the returned string.
 */
char *build_file_path(const char *dir_path, const char *filename)
{
    size_t size = strlen(dir_path) + strlen(filename) + 2;
    char *filepath = malloc(size);

    if (filepath == NULL)
    {
        return NULL; // Allocation failed
    }

    snprintf(filepath, size, "%s%s%s", dir_path, PATH_SEP, filename);

    if (access(filepath, X_OK) == 0)
    {
        return filepath; // Executable file found
    }

    free(filepath);
    return NULL;
}

/**
 * Checks if a file with the given name and extension exists in the directory.
 * Returns the full path if found and executable, NULL otherwise.
 */
char *check_file_with_extension(const char *dir_path, const char *filename, const char *extension, struct dirent *entry)
{
    char filename_with_ext[256];
    snprintf(filename_with_ext, sizeof(filename_with_ext), "%s%s", filename, extension);

    if (strcmp(entry->d_name, filename_with_ext) == 0)
    {
        return build_file_path(dir_path, entry->d_name);
    }

    return NULL;
}

/**
 * Searches for an executable file in multiple directories from a PATH string.
 * Looks for .exe and .bat extensions on Windows.
 * Returns dynamically allocated full path if found, NULL otherwise.
 * Caller must free the returned string.
 */
char *recursive_file_search(char fullpath[], char filename[])
{
    char paths[MAX_NO_OF_PATHS][MAX_PATH_LEN];
    char *token = strtok(fullpath, ";");
    int i = 0;

    // Split PATH into individual directories
    while (token != NULL && i < MAX_NO_OF_PATHS)
    {
        strcpy(paths[i], token);
        i++;
        token = strtok(NULL, ";");
    }

    int path_count = i;

    // Define extensions to search for
    const char *extensions[] = {".exe", ".bat"};
    const int ext_count = 2;

    // Search each directory
    for (i = 0; i < path_count; i++)
    {
        DIR *dir = opendir(paths[i]);
        if (dir == NULL)
        {
            continue; // Skip directories that can't be opened
        }

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
        {
            // Try each extension
            for (int j = 0; j < ext_count; j++)
            {
                char *filepath = check_file_with_extension(paths[i], filename, extensions[j], entry);
                if (filepath != NULL)
                {
                    closedir(dir);
                    return filepath;
                }
            }
        }

        closedir(dir);
    }

    return NULL; // Not found
}

char *find_file(char filename[])
{
    char *fullpath = get_var("PATH");

    if (fullpath != NULL)
    {
        char *filepath = recursive_file_search(fullpath, filename);
        if (filepath != NULL)
        {
            free(fullpath);
            return filepath; // free it after use
        }

        free(fullpath);
        return NULL;
    }
}

char *get_var(const char *var)
{
    const char *env_v = getenv(var);
    if (env_v != NULL)
    {
        char *env_v_copy = malloc(strlen(env_v) + 1);
        if (env_v_copy != NULL)
        {
            strcpy(env_v_copy, env_v);
            return env_v_copy;
        }
    }
    return NULL;
}