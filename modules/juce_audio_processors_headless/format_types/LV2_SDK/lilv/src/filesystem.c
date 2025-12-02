/*
  Copyright 2007-2021 David Robillard <d@drobilla.net>

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THIS SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#define _POSIX_C_SOURCE 200809L /* for fileno */
#define _BSD_SOURCE 1           /* for realpath, symlink */
#define _DEFAULT_SOURCE 1       /* for realpath, symlink */

#ifdef __APPLE__
#  define _DARWIN_C_SOURCE 1 /* for flock */
#endif

#include "filesystem.h"
#include "lilv_config.h"
#include "lilv_internal.h"

#ifdef _WIN32
#  include <direct.h>
#  include <io.h>
#  include <windows.h>
#  define F_OK 0
#  define mkdir(path, flags) _mkdir(path)
#  define S_ISDIR(mode) (((mode)&S_IFMT) == S_IFDIR)
#else
#  include <dirent.h>
#  include <unistd.h>
#endif

#if USE_FLOCK && USE_FILENO
#  include <sys/file.h>
#endif

#include <sys/stat.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef PAGE_SIZE
#  define PAGE_SIZE 4096
#endif

static bool
lilv_is_dir_sep(const char c)
{
  return c == '/' || c == LILV_DIR_SEP[0];
}

#ifdef _WIN32
static inline bool
is_windows_path(const char* path)
{
  return (isalpha(path[0]) && (path[1] == ':' || path[1] == '|') &&
          (path[2] == '/' || path[2] == '\\'));
}
#endif

char*
lilv_temp_directory_path(void)
{
#ifdef _WIN32
  DWORD len = GetTempPath(0, NULL);
  char* buf = (char*)calloc(len, 1);
  if (GetTempPath(len, buf) == 0) {
    free(buf);
    return NULL;
  }

  return buf;
#else
  const char* const tmpdir = getenv("TMPDIR");

  return tmpdir ? lilv_strdup(tmpdir) : lilv_strdup("/tmp");
#endif
}

bool
lilv_path_is_absolute(const char* path)
{
  if (lilv_is_dir_sep(path[0])) {
    return true;
  }

#ifdef _WIN32
  if (is_windows_path(path)) {
    return true;
  }
#endif

  return false;
}

bool
lilv_path_is_child(const char* path, const char* dir)
{
  if (path && dir) {
    const size_t path_len = strlen(path);
    const size_t dir_len  = strlen(dir);
    return dir && path_len >= dir_len && !strncmp(path, dir, dir_len);
  }
  return false;
}

char*
lilv_path_current(void)
{
  return getcwd(NULL, 0);
}

char*
lilv_path_absolute(const char* path)
{
  if (lilv_path_is_absolute(path)) {
    return lilv_strdup(path);
  }

  char* cwd      = getcwd(NULL, 0);
  char* abs_path = lilv_path_join(cwd, path);
  free(cwd);
  return abs_path;
}

char*
lilv_path_absolute_child(const char* path, const char* parent)
{
  if (lilv_path_is_absolute(path)) {
    return lilv_strdup(path);
  }

  return lilv_path_join(parent, path);
}

char*
lilv_path_relative_to(const char* path, const char* base)
{
  const size_t path_len = strlen(path);
  const size_t base_len = strlen(base);
  const size_t min_len  = (path_len < base_len) ? path_len : base_len;

  // Find the last separator common to both paths
  size_t last_shared_sep = 0;
  for (size_t i = 0; i < min_len && path[i] == base[i]; ++i) {
    if (lilv_is_dir_sep(path[i])) {
      last_shared_sep = i;
    }
  }

  if (last_shared_sep == 0) {
    // No common components, return path
    return lilv_strdup(path);
  }

  // Find the number of up references ("..") required
  size_t up = 0;
  for (size_t i = last_shared_sep + 1; i < base_len; ++i) {
    if (lilv_is_dir_sep(base[i])) {
      ++up;
    }
  }

#ifdef _WIN32
  const bool use_slash = strchr(path, '/');
#else
  static const bool use_slash = true;
#endif

  // Write up references
  const size_t suffix_len = path_len - last_shared_sep;
  char*        rel        = (char*)calloc(1, suffix_len + (up * 3) + 1);
  for (size_t i = 0; i < up; ++i) {
    if (use_slash) {
      memcpy(rel + (i * 3), "../", 3);
    } else {
      memcpy(rel + (i * 3), "..\\", 3);
    }
  }

  // Write suffix
  memcpy(rel + (up * 3), path + last_shared_sep + 1, suffix_len);
  return rel;
}

char*
lilv_path_parent(const char* path)
{
  const char* s = path + strlen(path) - 1; // Last character

  // Last non-slash
  for (; s > path && lilv_is_dir_sep(*s); --s) {
  }

  // Last internal slash
  for (; s > path && !lilv_is_dir_sep(*s); --s) {
  }

  // Skip duplicates
  for (; s > path && lilv_is_dir_sep(*s); --s) {
  }

  if (s == path) { // Hit beginning
    return lilv_is_dir_sep(*s) ? lilv_strdup("/") : lilv_strdup(".");
  }

  // Pointing to the last character of the result (inclusive)
  char* dirname = (char*)malloc(s - path + 2);
  memcpy(dirname, path, s - path + 1);
  dirname[s - path + 1] = '\0';
  return dirname;
}

char*
lilv_path_filename(const char* path)
{
  const size_t path_len = strlen(path);
  size_t       last_sep = path_len;
  for (size_t i = 0; i < path_len; ++i) {
    if (lilv_is_dir_sep(path[i])) {
      last_sep = i;
    }
  }

  if (last_sep >= path_len) {
    return lilv_strdup(path);
  }

  const size_t ret_len = path_len - last_sep;
  char* const  ret     = (char*)calloc(ret_len + 1, 1);

  strncpy(ret, path + last_sep + 1, ret_len);
  return ret;
}

char*
lilv_path_join(const char* a, const char* b)
{
  if (!a) {
    return (b && b[0]) ? lilv_strdup(b) : NULL;
  }

  const size_t a_len        = strlen(a);
  const size_t b_len        = b ? strlen(b) : 0;
  const bool   a_end_is_sep = a_len > 0 && lilv_is_dir_sep(a[a_len - 1]);
  const size_t pre_len      = a_len - (a_end_is_sep ? 1 : 0);
  char*        path         = (char*)calloc(1, a_len + b_len + 2);
  memcpy(path, a, pre_len);

#ifdef _WIN32
  // Use forward slash if it seems that the input paths do
  const bool a_has_slash = strchr(a, '/');
  const bool b_has_slash = b && strchr(b, '/');
  if (a_has_slash || b_has_slash) {
    path[pre_len] = '/';
  } else {
    path[pre_len] = '\\';
  }
#else
  path[pre_len] = '/';
#endif

  if (b) {
    memcpy(path + pre_len + 1,
           b + (lilv_is_dir_sep(b[0]) ? 1 : 0),
           lilv_is_dir_sep(b[0]) ? b_len - 1 : b_len);
  }
  return path;
}

char*
lilv_path_canonical(const char* path)
{
  if (!path) {
    return NULL;
  }

#if defined(_WIN32)
  char* out = (char*)malloc(MAX_PATH);
  GetFullPathName(path, MAX_PATH, out, NULL);
  return out;
#else
  char* real_path = realpath(path, NULL);
  return real_path ? real_path : lilv_strdup(path);
#endif
}

bool
lilv_path_exists(const char* path)
{
#if USE_LSTAT
  struct stat st;
  return !lstat(path, &st);
#else
  return !access(path, F_OK);
#endif
}

bool
lilv_is_directory(const char* path)
{
  struct stat st;
  return !stat(path, &st) && S_ISDIR(st.st_mode);
}

int
lilv_copy_file(const char* src, const char* dst)
{
  FILE* in = fopen(src, "r");
  if (!in) {
    return errno;
  }

  FILE* out = fopen(dst, "w");
  if (!out) {
    fclose(in);
    return errno;
  }

  char*  page   = (char*)malloc(PAGE_SIZE);
  size_t n_read = 0;
  int    st     = 0;
  while ((n_read = fread(page, 1, PAGE_SIZE, in)) > 0) {
    if (fwrite(page, 1, n_read, out) != n_read) {
      st = errno;
      break;
    }
  }

  if (!st && fflush(out)) {
    st = errno;
  }

  if (!st && (ferror(in) || ferror(out))) {
    st = EBADF;
  }

  free(page);
  fclose(in);
  fclose(out);

  return st;
}

int
lilv_symlink(const char* oldpath, const char* newpath)
{
  int ret = 0;
  if (strcmp(oldpath, newpath)) {
#ifdef _WIN32
    ret = !CreateHardLink(newpath, oldpath, 0);
#else
    char* target = lilv_path_relative_to(oldpath, newpath);

    ret = symlink(target, newpath);

    free(target);
#endif
  }
  return ret;
}

int
lilv_flock(FILE* file, bool lock, bool block)
{
#ifdef _WIN32
  HANDLE     handle     = (HANDLE)_get_osfhandle(fileno(file));
  OVERLAPPED overlapped = {0};

  if (lock) {
    const DWORD flags =
      (LOCKFILE_EXCLUSIVE_LOCK | (block ? 0 : LOCKFILE_FAIL_IMMEDIATELY));

    return !LockFileEx(handle, flags, 0, UINT32_MAX, UINT32_MAX, &overlapped);
  } else {
    return !UnlockFileEx(handle, 0, UINT32_MAX, UINT32_MAX, &overlapped);
  }
#elif USE_FLOCK && USE_FILENO
  return flock(fileno(file),
               (lock ? LOCK_EX : LOCK_UN) | (block ? 0 : LOCK_NB));
#else
  return 0;
#endif
}

void
lilv_dir_for_each(const char* path,
                  void*       data,
                  void (*f)(const char* path, const char* name, void* data))
{
#ifdef _WIN32
  char*           pat = lilv_path_join(path, "*");
  WIN32_FIND_DATA fd;
  HANDLE          fh = FindFirstFile(pat, &fd);
  if (fh != INVALID_HANDLE_VALUE) {
    do {
      if (strcmp(fd.cFileName, ".") && strcmp(fd.cFileName, "..")) {
        f(path, fd.cFileName, data);
      }
    } while (FindNextFile(fh, &fd));
  }
  FindClose(fh);
  free(pat);
#else
  DIR* dir = opendir(path);
  if (dir) {
    for (struct dirent* entry = NULL; (entry = readdir(dir));) {
      if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) {
        f(path, entry->d_name, data);
      }
    }
    closedir(dir);
  }
#endif
}

char*
lilv_create_temporary_directory_in(const char* pattern, const char* parent)
{
#ifdef _WIN32
  static const char chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static const int  n_chars = sizeof(chars) - 1;

  const size_t pattern_len = strlen(pattern);
  if (pattern_len < 7 || strcmp(pattern + pattern_len - 6, "XXXXXX")) {
    errno = EINVAL;
    return NULL;
  }

  char* const  path_pattern     = lilv_path_join(parent, pattern);
  const size_t path_pattern_len = strlen(path_pattern);
  char* const  suffix           = path_pattern + path_pattern_len - 6;

  for (unsigned attempt = 0; attempt < 128; ++attempt) {
    for (unsigned i = 0; i < 6; ++i) {
      suffix[i] = chars[rand() % n_chars];
    }

    if (!mkdir(path_pattern, 0700)) {
      return path_pattern;
    }
  }

  return NULL;
#else
  char* const path_pattern = lilv_path_join(parent, pattern);

  return mkdtemp(path_pattern); // NOLINT (not a leak)
#endif
}

char*
lilv_create_temporary_directory(const char* pattern)
{
  char* const tmpdir = lilv_temp_directory_path();
  char* const result = lilv_create_temporary_directory_in(pattern, tmpdir);

  free(tmpdir);

  return result;
}

int
lilv_create_directories(const char* dir_path)
{
  char*        path     = lilv_strdup(dir_path);
  const size_t path_len = strlen(path);
  size_t       i        = 1;

#ifdef _WIN32
  if (is_windows_path(dir_path)) {
    i = 3;
  }
#endif

  for (; i <= path_len; ++i) {
    const char c = path[i];
    if (c == LILV_DIR_SEP[0] || c == '/' || c == '\0') {
      path[i] = '\0';
      if (mkdir(path, 0755) && (errno != EEXIST || !lilv_is_directory(path))) {
        free(path);
        return errno;
      }
      path[i] = c;
    }
  }

  free(path);
  return 0;
}

static off_t
lilv_file_size(const char* path)
{
  struct stat buf;
  if (stat(path, &buf)) {
    return 0;
  }
  return buf.st_size;
}

int
lilv_remove(const char* path)
{
#ifdef _WIN32
  if (lilv_is_directory(path)) {
    return !RemoveDirectory(path);
  }
#endif

  return remove(path);
}

bool
lilv_file_equals(const char* a_path, const char* b_path)
{
  if (!strcmp(a_path, b_path)) {
    return true; // Paths match
  }

  bool        match  = false;
  FILE*       a_file = NULL;
  FILE*       b_file = NULL;
  char* const a_real = lilv_path_canonical(a_path);
  char* const b_real = lilv_path_canonical(b_path);
  if (!strcmp(a_real, b_real)) {
    match = true; // Real paths match
  } else if (lilv_file_size(a_path) != lilv_file_size(b_path)) {
    match = false; // Sizes differ
  } else if (!(a_file = fopen(a_real, "rb")) ||
             !(b_file = fopen(b_real, "rb"))) {
    match = false; // Missing file matches nothing
  } else {
    // TODO: Improve performance by reading chunks
    match = true;
    while (!feof(a_file) && !feof(b_file)) {
      if (fgetc(a_file) != fgetc(b_file)) {
        match = false;
        break;
      }
    }
  }

  if (a_file) {
    fclose(a_file);
  }
  if (b_file) {
    fclose(b_file);
  }
  free(a_real);
  free(b_real);
  return match;
}
