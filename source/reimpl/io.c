/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022      Rinnegatamante
 * Copyright (C) 2022-2024 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "reimpl/io.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdarg.h>
#include <psp2/kernel/threadmgr.h>

#ifdef USE_SCELIBC_IO
#include <libc_bridge/libc_bridge.h>
#endif

#include <fios/fios.h>

#include "utils/logger.h"
#include "utils/utils.h"

// Includes the following inline utilities:
// int oflags_musl_to_newlib(int flags);
// dirent64_bionic * dirent_newlib_to_bionic(struct dirent* dirent_newlib);
// void stat_newlib_to_bionic(struct stat * src, stat64_bionic * dst);
#include "reimpl/bits/_struct_converters.c"

#include "_existing_files.c"

static int compare_strings(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

FILE * fopen_soloader(const char * filename, const char * mode) {
    if (strchr(filename, '/') == filename) {
        char **existing_file = bsearch(&filename, existing_files, existing_files_len, sizeof(existing_files[0]), compare_strings);
        if (existing_file) {
            FILE * f = NULL;

            if (sceFiosFHOpenSync(NULL, (int32_t*)&f, filename, NULL)) {
                l_warn("fopen<psarc>(%s, %s): not found / 0x%x", filename, mode, f);
                return NULL;
            } else {
                //l_debug("fopen<psarc>(%s, %s): 0x%x", filename, mode, f);
                return f;
            }
        }
        return NULL;
    }

#ifdef USE_SCELIBC_IO
    FILE* ret = sceLibcBridge_fopen(filename, mode);
#else
    FILE* ret = fopen(filename, mode);
#endif

    if (ret)
        l_debug("fopen(%s, %s): %p", filename, mode, ret);
    else
        l_warn("fopen(%s, %s): %p", filename, mode, ret);

    return ret;
}

int open_soloader(const char * path, int oflag, ...) {
    if (strcmp(path, "/proc/cpuinfo") == 0) {
        return open_soloader("app0:/cpuinfo", oflag);
    } else if (strcmp(path, "/proc/meminfo") == 0) {
        return open_soloader("app0:/meminfo", oflag);
    } else if (strcmp(path, "/dev/urandom") == 0) {
        return open_soloader("app0:/urandom", oflag);
    }

    mode_t mode = 0666;
    if (((oflag & BIONIC_O_CREAT) == BIONIC_O_CREAT) ||
        ((oflag & BIONIC_O_TMPFILE) == BIONIC_O_TMPFILE)) {
        va_list args;
        va_start(args, oflag);
        mode = (mode_t)(va_arg(args, int));
        va_end(args);
    }

    oflag = oflags_bionic_to_newlib(oflag);
    int ret = open(path, oflag, mode);
    if (ret >= 0)
        l_debug("open(%s, %x): %i", path, oflag, ret);
    else
        l_warn("open(%s, %x): %i", path, oflag, ret);
    return ret;
}

int fstat_soloader(int fd, stat64_bionic * buf) {
    struct stat st;
    int res = fstat(fd, &st);

    if (res == 0)
        stat_newlib_to_bionic(&st, buf);

    l_debug("fstat(%i): %i", fd, res);
    return res;
}

int stat_soloader(const char * path, stat64_bionic * buf) {
    if (strcmp(path, "/system/lib/libOpenSLES.so") == 0) {
        l_debug("stat(%s): returning 0 in case this is a check for OpenSLES support", path);
        return 0;
    }

    struct stat st;
    int res = stat(path, &st);

    if (res == 0)
        stat_newlib_to_bionic(&st, buf);

    l_debug("stat(%s): %i", path, res);
    return res;
}

int fclose_soloader(FILE * f) {
    //l_debug("fclose(%p)", f);

    if ((uintptr_t)f > 0x81000000) {
#ifdef USE_SCELIBC_IO
        int ret = sceLibcBridge_fclose(f);
#else
        int ret = fclose(f);
#endif

        return ret;
    }

    sceFiosFHCloseSync(NULL, (int32_t)f);
}

int close_soloader(int fd) {
    int ret = close(fd);
    l_debug("close(%i): %i", fd, ret);
    return ret;
}

DIR* opendir_soloader(char* _pathname) {
    DIR* ret = opendir(_pathname);
    l_debug("opendir(\"%s\"): %p", _pathname, ret);
    return ret;
}

struct dirent64_bionic * readdir_soloader(DIR * dir) {
    static struct dirent64_bionic dirent_tmp;

    struct dirent* ret = readdir(dir);
    l_debug("readdir(%p): %p", dir, ret);

    if (ret) {
        dirent64_bionic* entry_tmp = dirent_newlib_to_bionic(ret);
        memcpy(&dirent_tmp, entry_tmp, sizeof(dirent64_bionic));
        free(entry_tmp);
        return &dirent_tmp;
    }

    return NULL;
}

int readdir_r_soloader(DIR * dirp, dirent64_bionic * entry,
                       dirent64_bionic ** result) {
    struct dirent dirent_tmp;
    struct dirent * pdirent_tmp;

    int ret = readdir_r(dirp, &dirent_tmp, &pdirent_tmp);

    if (ret == 0) {
        dirent64_bionic* entry_tmp = dirent_newlib_to_bionic(&dirent_tmp);
        memcpy(entry, entry_tmp, sizeof(dirent64_bionic));
        *result = (pdirent_tmp != NULL) ? entry : NULL;
        free(entry_tmp);
    }

    l_debug("readdir_r(%p, %p, %p): %i", dirp, entry, result, ret);
    return ret;
}

int closedir_soloader(DIR * dir) {
    int ret = closedir(dir);
    l_debug("closedir(%p): %i", dir, ret);
    return ret;
}

int fcntl_soloader(int fd, int cmd, ...) {
    l_warn("fcntl(%i, %i, ...): not implemented", fd, cmd);
    return 0;
}

int ioctl_soloader(int fd, int request, ...) {
    l_warn("ioctl(%i, %i, ...): not implemented", fd, request);
    return 0;
}

int fsync_soloader(int fd) {
    int ret = fsync(fd);
    l_debug("fsync(%i): %i", fd, ret);
    return ret;
}

int statfs_soloader(const char *path, struct statfs *buf) {
    l_debug("statfs(\"%s\", %p)");

    if (!buf)
        return -1;

    buf->f_type = EXT4_SUPER_MAGIC;
    //TODO: can do actual values here at some point
    buf->f_blocks = 10000000;
    buf->f_bfree = 10000000;
    buf->f_bavail = 10000000;
    buf->f_files = 10000000;
    buf->f_ffree = 10000000;
    buf->f_bsize = 1024;
    buf->f_frsize = 1024;

    return 0;
}

int fseek_soloader(FILE *stream, long int offset, int origin) {
    //l_debug("fseek(%p, %i, %i)", stream, offset, origin);
    if ((uintptr_t)stream > 0x81000000)
#ifdef USE_SCELIBC_IO
        return sceLibcBridge_fseek(stream, offset, origin);
#else
        return fseek(stream, offset, origin);
#endif

    sceFiosFHSeek((int32_t)stream, offset, origin);
    return 0;
}

size_t fread_soloader(void *ptr, size_t size, size_t count, FILE *stream) {
    //l_debug("fread_soloader(%p, %i, %i, %i)", ptr, size, count, stream);

    if ((uintptr_t)stream > 0x81000000)
#ifdef USE_SCELIBC_IO
        return sceLibcBridge_fread(ptr, size, count, stream);
#else
        return fread(ptr, size, count, stream);
#endif

    sceFiosFHReadSync(NULL, (int32_t)stream, ptr, size * count);

    return count;
}

long int ftell_soloader(FILE *stream) {
    //l_debug("ftell_soloader(%p)", stream);

    if ((uintptr_t)stream > 0x81000000)
#ifdef USE_SCELIBC_IO
        return sceLibcBridge_ftell(stream);
#else
        return ftell(stream);
#endif

    return sceFiosFHTell((int32_t)stream);
}

int feof_soloader(FILE *stream) {
    //l_debug("feof_soloader(%p)", stream);

    if ((uintptr_t)stream > 0x81000000)
#ifdef USE_SCELIBC_IO
        return sceLibcBridge_feof(stream);
#else
        return feof(stream);
#endif

    return 0;
}

int ferror_soloader(FILE *stream) {
    l_debug("ferror_soloader(%p)", stream);
    if ((uintptr_t)stream > 0x81000000)
#ifdef USE_SCELIBC_IO
        return sceLibcBridge_ferror(stream);
#else
        return ferror(stream);
#endif

    return 0;
}

int fflush_soloader(FILE *stream) {
    if ((uintptr_t)stream > 0x81000000)
#ifdef USE_SCELIBC_IO
        return sceLibcBridge_fflush(stream);
#else
        return fflush(stream);
#endif

    return 0;
}

int fgetc_soloader(FILE *stream) {
    if ((uintptr_t)stream > 0x81000000)
#ifdef USE_SCELIBC_IO
        return sceLibcBridge_fgetc(stream);
#else
        return fgetc(stream);
#endif

    return 0;
}
