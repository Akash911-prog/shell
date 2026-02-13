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

// constants
#define MAX_PATH_LEN 1024  // len of path string size
#define MAX_NO_OF_PATHS 30 // no of paths allowed in the full path

char *recursive_file_search(char fullpath[], char filename[])
{
    // Array to store individual directory paths after splitting
    char paths[MAX_NO_OF_PATHS][MAX_PATH_LEN];

    // Split the fullpath string on semicolon delimiters
    char *token = strtok(fullpath, ";");
    int i = 0;

    // Extract each directory path and store it in the paths array
    while (token != NULL && i < MAX_NO_OF_PATHS)
    {
        strcpy(paths[i], token);
        i++;
        token = strtok(NULL, ";");
    }

    int path_count = i; // Number of directories to search
    i = 0;

    // Iterate through each directory path
    while (i < path_count)
    {
        // Attempt to open the directory for reading
        DIR *dir = opendir(paths[i]);
        if (dir != NULL)
        {
            struct dirent *entry;

            // Read each entry in the directory
            while ((entry = readdir(dir)) != NULL)
            {
                // Build the filename with .exe extension
                char filename_with_exe[256];
                snprintf(filename_with_exe, sizeof(filename_with_exe), "%s%s", filename, ".exe");

                // Calculate size needed for full file path
                size_t size = (strlen(paths[i]) + strlen(entry->d_name) + 2);

                // Check if current entry matches the target filename
                if (strcmp((entry->d_name), filename_with_exe) == 0)
                {
                    closedir(dir);

                    // Allocate memory for the complete file path
                    char *filepath;
                    filepath = malloc(size);

                    // Construct full path: directory + separator + filename
                    snprintf(filepath, size, "%s%s%s", paths[i], PATH_SEP, entry->d_name);

                    // Verify the file is executable
                    if (access(filepath, X_OK) == 0)
                    {
                        return filepath; // Found and executable
                    }

                    // File exists but not executable, continue searching
                    free(filepath);
                    continue;
                }
            }
            closedir(dir);
        }
        i++;
    }

    // File not found in any directory
    return NULL;
}