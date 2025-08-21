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
        return NULL;char* make_path(const char* relative_path) {
    char* absolute = malloc(PATH_MAX);
    if (!absolute) return NULL;
    path_make_absolute(absolute, relative_path);
    return absolute;
}

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

int path_make_absolute(char* out_path, const char* relative_path) {
	if (!out_path || !relative_path) {
        fprintf(stderr, "FILEPATH ERROR: relative_path or out_pathpath is NULL\n");
        return -1;
    }

    char* self_dir = get_self_directory();
    if (!self_dir) return -1;

    char* base = get_parent_directory(self_dir);
    if (!base) return -1;

#if defined(_WIN32)
    snprintf(out_path, 1024, "%s\\%s", base, relative_path);
#else
    snprintf(out_path, 1024, "%s/%s", base, relative_path);
#endif

	free(self_dir);
	free(base);
	return 0;
}

char* make_path(char* path) {
    char* absolute = malloc(1024);
    if (!absolute) return NULL;
    path_make_absolute(absolute, path);
    return absolute;
}

#endif // FILEPATH_H

