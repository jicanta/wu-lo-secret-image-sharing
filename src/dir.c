#define _POSIX_C_SOURCE 200809L

#include "dir.h"

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int has_bmp_ext(const char* name) {
  size_t len = strlen(name);
  if (len < 4) return 0;
  const char* ext = name + len - 4;
  return ext[0] == '.' && (ext[1] == 'b' || ext[1] == 'B') &&
         (ext[2] == 'm' || ext[2] == 'M') && (ext[3] == 'p' || ext[3] == 'P');
}

static int cmp_str(const void* a, const void* b) {
  return strcmp(*(const char* const*)a, *(const char* const*)b);
}

int dir_list_bmps(const char* dir, char*** out_paths, int* out_count) {
  const char* base = dir ? dir : ".";
  DIR* d = opendir(base);
  if (!d) {
    fprintf(stderr, "Error: cannot open directory '%s'.\n", base);
    return -1;
  }

  char** paths = NULL;
  int count = 0, cap = 0;
  struct dirent* e;
  while ((e = readdir(d)) != NULL) {
    if (!has_bmp_ext(e->d_name)) continue;
    if (count == cap) {
      cap = cap ? cap * 2 : 8;
      char** grown = realloc(paths, (size_t)cap * sizeof(char*));
      if (!grown) goto fail;
      paths = grown;
    }
    size_t len = strlen(base) + 1 + strlen(e->d_name) + 1;
    char* path = malloc(len);
    if (!path) goto fail;
    snprintf(path, len, "%s/%s", base, e->d_name);
    paths[count++] = path;
  }
  closedir(d);

  qsort(paths, (size_t)count, sizeof(char*), cmp_str);
  *out_paths = paths;
  *out_count = count;
  return 0;

fail:
  fprintf(stderr, "Error: out of memory.\n");
  for (int i = 0; i < count; i++) free(paths[i]);
  free(paths);
  closedir(d);
  return -1;
}

void dir_free(char** paths, int count) {
  for (int i = 0; i < count; i++) free(paths[i]);
  free(paths);
}
