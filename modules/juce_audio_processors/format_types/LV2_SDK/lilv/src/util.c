/*
  Copyright 2007-2019 David Robillard <d@drobilla.net>

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

#include "filesystem.h"
#include "lilv_internal.h"

#include "lilv/lilv.h"
#include "serd/serd.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
lilv_free(void* ptr)
{
  free(ptr);
}

char*
lilv_strjoin(const char* first, ...)
{
  size_t len    = strlen(first);
  char*  result = (char*)malloc(len + 1);

  memcpy(result, first, len);

  va_list args;
  va_start(args, first);
  while (1) {
    const char* const s = va_arg(args, const char*);
    if (s == NULL) {
      break;
    }

    const size_t this_len   = strlen(s);
    char*        new_result = (char*)realloc(result, len + this_len + 1);
    if (!new_result) {
      va_end(args);
      free(result);
      return NULL;
    }

    result = new_result;
    memcpy(result + len, s, this_len);
    len += this_len;
  }
  va_end(args);

  result[len] = '\0';

  return result;
}

char*
lilv_strdup(const char* str)
{
  if (!str) {
    return NULL;
  }

  const size_t len  = strlen(str);
  char*        copy = (char*)malloc(len + 1);
  memcpy(copy, str, len + 1);
  return copy;
}

const char*
lilv_uri_to_path(const char* uri)
{
#if defined(__GNUC__) && __GNUC__ > 4
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

  return (const char*)serd_uri_to_path((const uint8_t*)uri);

#if defined(__GNUC__) && __GNUC__ > 4
#  pragma GCC diagnostic pop
#endif
}

char*
lilv_file_uri_parse(const char* uri, char** hostname)
{
  return (char*)serd_file_uri_parse((const uint8_t*)uri, (uint8_t**)hostname);
}

/** Return the current LANG converted to Turtle (i.e. RFC3066) style.
 * For example, if LANG is set to "en_CA.utf-8", this returns "en-ca".
 */
char*
lilv_get_lang(void)
{
  const char* const env_lang = getenv("LANG");
  if (!env_lang || !strcmp(env_lang, "") || !strcmp(env_lang, "C") ||
      !strcmp(env_lang, "POSIX")) {
    return NULL;
  }

  const size_t env_lang_len = strlen(env_lang);
  char* const  lang         = (char*)malloc(env_lang_len + 1);
  for (size_t i = 0; i < env_lang_len + 1; ++i) {
    if (env_lang[i] == '_') {
      lang[i] = '-'; // Convert _ to -
    } else if (env_lang[i] >= 'A' && env_lang[i] <= 'Z') {
      lang[i] = env_lang[i] + ('a' - 'A'); // Convert to lowercase
    } else if (env_lang[i] >= 'a' && env_lang[i] <= 'z') {
      lang[i] = env_lang[i]; // Lowercase letter, copy verbatim
    } else if (env_lang[i] >= '0' && env_lang[i] <= '9') {
      lang[i] = env_lang[i]; // Digit, copy verbatim
    } else if (env_lang[i] == '\0' || env_lang[i] == '.') {
      // End, or start of suffix (e.g. en_CA.utf-8), finished
      lang[i] = '\0';
      break;
    } else {
      LILV_ERRORF("Illegal LANG `%s' ignored\n", env_lang);
      free(lang);
      return NULL;
    }
  }

  return lang;
}

#ifndef _WIN32

/** Append suffix to dst, update dst_len, and return the realloc'd result. */
static char*
strappend(char* dst, size_t* dst_len, const char* suffix, size_t suffix_len)
{
  dst = (char*)realloc(dst, *dst_len + suffix_len + 1);
  memcpy(dst + *dst_len, suffix, suffix_len);
  dst[(*dst_len += suffix_len)] = '\0';
  return dst;
}

/** Append the value of the environment variable var to dst. */
static char*
append_var(char* dst, size_t* dst_len, const char* var)
{
  // Get value from environment
  const char* val = getenv(var);
  if (val) { // Value found, append it
    return strappend(dst, dst_len, val, strlen(val));
  }

  // No value found, append variable reference as-is
  return strappend(strappend(dst, dst_len, "$", 1), dst_len, var, strlen(var));
}

#endif

/** Expand variables (e.g. POSIX ~ or $FOO, Windows %FOO%) in `path`. */
char*
lilv_expand(const char* path)
{
#ifdef _WIN32
  char* out = (char*)malloc(MAX_PATH);
  ExpandEnvironmentStrings(path, out, MAX_PATH);
#else
  char*  out = NULL;
  size_t len = 0;

  const char* start = path; // Start of current chunk to copy
  for (const char* s = path; *s;) {
    if (*s == '$') {
      // Hit $ (variable reference, e.g. $VAR_NAME)
      for (const char* t = s + 1;; ++t) {
        if (!*t || (!isupper(*t) && !isdigit(*t) && *t != '_')) {
          // Append preceding chunk
          out = strappend(out, &len, start, s - start);

          // Append variable value (or $VAR_NAME if not found)
          char* var = (char*)calloc(t - s, 1);
          memcpy(var, s + 1, t - s - 1);
          out = append_var(out, &len, var);
          free(var);

          // Continue after variable reference
          start = s = t;
          break;
        }
      }
    } else if (*s == '~' && (*(s + 1) == '/' || !*(s + 1))) {
      // Hit ~ before slash or end of string (home directory reference)
      out   = strappend(out, &len, start, s - start);
      out   = append_var(out, &len, "HOME");
      start = ++s;
    } else {
      ++s;
    }
  }

  if (*start) {
    out = strappend(out, &len, start, strlen(start));
  }
#endif

  return out;
}

char*
lilv_find_free_path(const char* in_path,
                    bool (*exists)(const char*, const void*),
                    const void* user_data)
{
  const size_t in_path_len = strlen(in_path);
  char*        path        = (char*)malloc(in_path_len + 7);
  memcpy(path, in_path, in_path_len + 1);

  for (unsigned i = 2; i < 1000000u; ++i) {
    if (!exists(path, user_data)) {
      return path;
    }
    snprintf(path, in_path_len + 7, "%s.%u", in_path, i);
  }

  return NULL;
}

typedef struct {
  char*  pattern;
  time_t time;
  char*  latest;
} Latest;

static void
update_latest(const char* path, const char* name, void* data)
{
  Latest*  latest     = (Latest*)data;
  char*    entry_path = lilv_path_join(path, name);
  unsigned num        = 0;
  if (sscanf(entry_path, latest->pattern, &num) == 1) {
    struct stat st;
    if (!stat(entry_path, &st)) {
      if (st.st_mtime >= latest->time) {
        free(latest->latest);
        latest->latest = entry_path;
      }
    } else {
      LILV_ERRORF("stat(%s) (%s)\n", path, strerror(errno));
    }
  }
  if (entry_path != latest->latest) {
    free(entry_path);
  }
}

/** Return the latest copy of the file at `path` that is newer. */
char*
lilv_get_latest_copy(const char* path, const char* copy_path)
{
  char*  copy_dir = lilv_path_parent(copy_path);
  Latest latest   = {lilv_strjoin(copy_path, ".%u", NULL), 0, NULL};

  struct stat st;
  if (!stat(path, &st)) {
    latest.time = st.st_mtime;
  } else {
    LILV_ERRORF("stat(%s) (%s)\n", path, strerror(errno));
  }

  lilv_dir_for_each(copy_dir, &latest, update_latest);

  free(latest.pattern);
  free(copy_dir);
  return latest.latest;
}
