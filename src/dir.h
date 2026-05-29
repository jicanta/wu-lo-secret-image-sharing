#ifndef DIR_H
#define DIR_H

int dir_list_bmps(const char* dir, char*** out_paths, int* out_count);

void dir_free(char** paths, int count);

#endif
