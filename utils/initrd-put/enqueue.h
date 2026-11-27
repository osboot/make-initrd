// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef MKRD_INITRD_PUT_ENQUEUE_H
#define MKRD_INITRD_PUT_ENQUEUE_H

#include <stdbool.h>
#include <limits.h>

bool is_shebang(char buf[LINE_MAX]);
int enqueue_shebang(const char *filename, char buf[LINE_MAX]);

void init_elf_library(void);
bool is_elf_file(char buf[LINE_MAX]);
int enqueue_libraries(const char *filename, int fd);
void elf_cache_destroy(void);

#endif // MKRD_INITRD_PUT_ENQUEUE_H

