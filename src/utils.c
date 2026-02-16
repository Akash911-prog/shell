#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <io.h>
#include "utils.h"
#include "variables.h"

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

void tokenize_cmd(char cmd[], char tokens[][50], int *no_of_tokens)
{
    int i = 0; // index of cmd
    int j = 0; // index of tokens
    int k = 0; // index inside current token

    int inside_quote = 0; // flag for inside_quotes state

    while (cmd[i] != '\0')
    {
        if (cmd[i] == '"')
        {
            inside_quote = !inside_quote;
            i++;
            continue;
        }

        // if whitespace and not inside quotes
        if ((cmd[i] == ' ') & !inside_quote)
        {
            // if index of token is > 0 then ends the ongoing token
            if (k > 0)
            {
                tokens[j][k] = '\0';
                j++;
                k = 0;
            }
            i++; // increase cmd index
            continue;
        }

        // adds the current cmd[i] to the current ongoing token
        tokens[j][k++] = cmd[i];
        i++;
    }

    // ends the last token if any
    if (k > 0)
    {
        tokens[j][k] = '\0';
        j++;
    }

    *no_of_tokens = j;
}

void set_pwd()
{
    char buffer[1024];
    if (GetCWD(buffer, FILENAME_MAX) != NULL)
    {
        Variables.set("PWD", buffer);
        return;
    }
    Variables.set("PWD", Variables.get("HOME"));
}

void set_username()
{
#ifdef _WIN32 // windows
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;

    if (GetUserName(username, &username_len))
    {
        Variables.set("USER", username);
    }
#else // linux/macos
    uid_t uid = geteuid();
    struct passwd *p = getpwuid(uid);

    if (p)
    {
        Variables.set("USER", p->pw_name);
    }
#endif
}

void set_hostname()
{
#ifdef _WIN32
    static char hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = MAX_COMPUTERNAME_LENGTH + 1;

    if (GetComputerName(hostname, &size))
    {
        Variables.set("HOST", hostname);
    }
#else
    static char hostname[256];

    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        Variables.set("HOST", hostname);
    }
#endif
}

void set_home_directory()
{
#ifdef _WIN32
    // Windows: Use USERPROFILE environment variable
    char *home = getenv("USERPROFILE");
    if (home == NULL)
    {
        // Fallback: combine HOMEDRIVE and HOMEPATH
        char *homedrive = getenv("HOMEDRIVE"); // Usually "C:"
        char *homepath = getenv("HOMEPATH");   // Usually "\Users\username"

        if (homedrive && homepath)
        {
            static char full_path[MAX_PATH];
            snprintf(full_path, MAX_PATH, "%s%s", homedrive, homepath);
            Variables.set("HOME", full_path);
            return;
        }
    }
    Variables.set("HOME", home);
#else
    // Unix/Linux/Mac: Use HOME environment variable
    char *home = getenv("HOME");
    if (home != NULL)
    {
        Variables.set("HOME", home);
        return;
    }

    // Fallback: Get from password database
    uid_t uid = getuid();
    struct passwd *pw = getpwuid(uid);
    if (pw)
    {
        Variables.set("HOME", pw->pw_dir);
    }
#endif
}

// Helper function to escape backslashes
void escape_backslashes(const char *src, char *dst, size_t dst_size)
{
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < dst_size - 2; i++)
    {
        if (src[i] == '\\')
        {
            dst[j++] = '\\'; // Add extra backslash
            dst[j++] = '\\';
        }
        else
        {
            dst[j++] = src[i];
        }
    }
    dst[j] = '\0';
}

void init_prompt()
{
    set_home_directory();
    set_pwd();
    set_username();
    set_hostname();

    char *host = Variables.get("HOST");
    char *pwd = Variables.get("PWD");
    char *user = Variables.get("USER");

    if (!user)
        user = "user";
    if (!host)
        host = "localhost";
    if (!pwd)
        pwd = "~";

    char buffer[2048];

    snprintf(buffer, sizeof(buffer), "\033[32m%s@\033[35m%s \033[36;1m%s\n$\033[33;22m ", user, host, pwd); // ansi color code

    Variables.set("PS1", buffer);
}