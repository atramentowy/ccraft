#ifndef FILEPATH_H
#define FILEPATH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux
#include <unistd.h>

char* get_self_directory(void) {
    char exe_path[1024];
    ssize_t path_length = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (path_length == -1)
        return NULL;

    exe_path[path_length] = '\0';

    char* last_slash = strrchr(exe_path, '/');
    if (last_slash)
        *last_slash = '\0';

    return strdup(exe_path);
}

#elif defined _WIN32
#include <Windows.h>

char* get_self_directory(void) {
    char exe_path[1024];
    DWORD path_length = GetModuleFileNameA(NULL, exe_path, sizeof(exe_path) - 1);
    if (path_length == 0)
        return NULL;

    exe_path[path_length] = '\0';

    char* last_backslash = strrchr(exe_path, '\\');
    if (last_backslash)
        *last_backslash = '\0';

    return _strdup(exe_path);
}

#else
#error get_self_directory not implemented for the current platform
#endif

char* get_parent_directory(const char* path) {
    if (!path) {
        fprintf(stderr, "FILEPATH ERROR: could not determine parent directory\n");
        return NULL;
    }

    char temp[1024];
    strncpy(temp, path, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

#if defined(_WIN32)
    char* last_sep = strrchr(temp, '\\');
#else
    char* last_sep = strrchr(temp, '/');
#endif

    if (!last_sep)
        return NULL;

    *last_sep = '\0';
    return strdup(temp);
}

char* get_absolute_path(const char* relative_path) {
    if (!relative_path) {
        fprintf(stderr, "FILEPATH ERROR: relative path is NULL\n");
        return NULL;
    }

    char* self_dir = get_self_directory();
    if (!self_dir)
        return NULL;

    char* base = get_parent_directory(self_dir);
    free(self_dir);

    if (!base)
        return NULL;

    size_t total_len = strlen(base) + strlen(relative_path) + 2;
    char* full_path = malloc(total_len);
    if (!full_path) {
        free(base);
        return NULL;
    }

#if defined(_WIN32)
    snprintf(full_path, total_len, "%s\\%s", base, relative_path);
#else
    snprintf(full_path, total_len, "%s/%s", base, relative_path);
#endif

    free(base);
    return full_path;
}

#endif // FILEPATH_H

