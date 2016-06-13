#ifndef INITRD_LS_H
#define INITRD_LS_H

#include "initrd-cpio.h"

int preformat(struct cpio_header *header);
int show_header(struct cpio_header *header);

#endif /* INITRD_LS_H */
