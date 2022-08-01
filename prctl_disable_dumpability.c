#define _GNU_SOURCE

#include <sys/prctl.h>

#ifndef QUIET
#include <stdio.h>
#endif
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>

int __attribute__((constructor)) disable_dumpability() {
    return prctl(PR_SET_DUMPABLE, 0); // 0=SUID_DUMP_DISABLE
}

//static int (*orig_execle)(const char *pathname, const char *arg, ...
//                       /*, (char *) NULL, char *const envp[] */) = NULL;
static int (*orig_execve)(const char *pathname, char *const argv[],
                  char *const envp[]) = NULL;
static int (*orig_execvpe)(const char *file, char *const argv[], char *const envp[]) = NULL;
static int (*orig_fexecve)(int fd, char *const argv[], char *const envp[]) = NULL;

static void* get_func_handle(const char* func_name) {
    dlerror();
    void * func_ptr = dlsym(RTLD_NEXT, func_name);
    char * dlsym_err = dlerror();
    if (dlsym_err != NULL) {
        #ifndef QUIET
        fprintf(stderr, "Error getting handle %s via `dlsym`: %s\n",
            func_name, dlsym_err);
        #endif
        exit(1);
    }
    return func_ptr;
}
static void __attribute__((constructor)) init_orig_handles()
{
    //orig_execle = get_func_handle("execle");
    orig_execve = get_func_handle("execve");
    orig_execvpe = get_func_handle("execvpe");
    orig_fexecve = get_func_handle("fexecve");
    #ifndef QUIET
    fputs("Init prctl_disable_dumpability success\n", stderr);
    #endif
}

static char** env_append(char *const envp[]) {
    char * const * envp_iter = envp;
    size_t envp_new_size = 0;

    // Check current envp size
    while (*envp_iter != NULL) {
        if (envp_new_size == INT_MAX) {
            errno = E2BIG;
            return NULL;
        }
        envp_iter++;
        envp_new_size++;
    }
    envp_new_size += 1;

    // Add 1 for the terminating NULL
    char ** envp_append = malloc((envp_new_size+1)*sizeof(char*));
    if (envp_append==NULL) {
        errno = E2BIG;
        return NULL;
    }
    envp_iter = envp;
    size_t envp_append_idx = 0;
    while (*envp_iter != NULL) {
        envp_append[envp_append_idx] = *envp_iter;
        envp_iter++;
        envp_append_idx++;
    }
    // Append the current environment LD_PRELOAD
    envp_append[envp_append_idx] = getenv("LD_PRELOAD");
    envp_append[envp_append_idx+1] = NULL;
    return envp_append;
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    char** new_envp = env_append(envp);
    if (new_envp == NULL) {
        return -1;
    }
    return orig_execve(pathname, argv, new_envp);
}
int execvpe(const char *file, char *const argv[], char *const envp[]) {
    char** new_envp = env_append(envp);
    if (new_envp == NULL) {
        return -1;
    }
    return orig_execvpe(file, argv, new_envp);
}
int execle(const char *pathname, const char *arg, ...) {
    // Convert the varargs into a vector and call execve instead
    va_list args_list;
    int argc = 1;
    va_start(args_list, arg);
    while (va_arg(args_list, const char*) != NULL) {
        if (argc == INT_MAX) {
            errno = E2BIG;
            return -1;
        }
        argc++;
    }
    va_end(args_list);

    // Add 1 for the terminating NULL
    char ** arg_vec = malloc((argc+1)*sizeof(char*));
    if (arg_vec==NULL) {
        errno = E2BIG;
        return -1;
    }
    arg_vec[0] = arg;
    va_start(args_list, arg);
    // Up to and including argc for ending NULL
    for (int arg_idx = 1; arg_idx <= argc; arg_idx++) {
        arg_vec[arg_idx] = va_arg(args_list, char*);
    }
    char * const * envp = va_arg(args_list, char**);
    va_end(args_list);

    return execve(pathname, arg_vec, envp);
}
int fexecve(int fd, char *const argv[], char *const envp[]) {
    char** new_envp = env_append(envp);
    if (new_envp == NULL) {
        return -1;
    }
    return orig_fexecve(fd, argv, new_envp);
}
