// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <endian.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <err.h>

#include "initrd-common.h"
#include "initrd-cpio.h"
#include "initrd-decompress.h"
#include "initrd-parse.h"

#define BOOTCONFIG_MAGIC "#BOOTCONFIG\n"
#define BOOTCONFIG_MAGIC_LEN 12

static int stream_level = 0;

void
read_stream(const char *compress, struct stream *arv, struct result *res)
{
	struct list_tail *l;
	struct stream *a;
	struct cpio *cpio;
	const char *compress_name;
	decompress_fn decompress;
	unsigned char *data = NULL;
	uint32_t *hdr;
	uint32_t size = 0; //csum;

	unsigned long offset = 0;

	stream_level++;

	if (stream_level == 1) {
		/*
		 * Check bootconfig first.
		 */
		data = arv->addr + arv->size - BOOTCONFIG_MAGIC_LEN;

		/*
		 * Since Grub may align the size of initrd to 4, we must
		 * check the preceding 3 bytes as well.
		 */
		for (int i = 0; i < 4; i++) {
			if (!memcmp(data, BOOTCONFIG_MAGIC, BOOTCONFIG_MAGIC_LEN))
				goto bootconfig;
			data--;
		}

		data = NULL;
	}
stream:
	while (offset < arv->size) {
		decompress = decompress_method(arv->addr + offset, arv->size - offset, &compress_name);
		if (decompress) {
			unsigned char *unpack     = NULL;
			unsigned long unpack_size = 0;
			unsigned long long readed = 0;

			//printf("Detected %s compressed data\n", compress_name);
			if (decompress(arv->addr + offset, arv->size - offset, &unpack, &unpack_size, &readed) != DECOMP_OK)
				err(EXIT_FAILURE, "ERROR: %s: %d: decompressor failed", __FILE__, __LINE__);

			l = list_append(&res->streams, sizeof(struct stream));
			if (l == NULL)
				err(EXIT_FAILURE, "ERROR: %s: %d: unable to add element to list", __FILE__, __LINE__);
			a = l->data;

			a->addr      = unpack;
			a->size      = unpack_size;
			a->allocated = 1;

			read_stream(compress_name, a, res);

			offset += readed;
			continue;
		}

		l = list_append(&res->cpios, sizeof(struct cpio));
		if (l == NULL)
			err(EXIT_FAILURE, "ERROR: %s: %d: unable to add element to list", __FILE__, __LINE__);
		cpio = l->data;

		cpio->type     = CPIO_ARCHIVE;
		cpio->compress = compress;
		cpio->raw      = arv->addr + offset;
		cpio->size     = arv->size - offset;
		cpio->headers  = NULL;

		offset += read_cpio(cpio);
	}

	if (data) {
		l = list_append(&res->cpios, sizeof(struct cpio));
		if (l == NULL)
			err(EXIT_FAILURE, "ERROR: %s: %d: unable to add element to list", __FILE__, __LINE__);

		cpio = l->data;

		cpio->type     = CPIO_BOOTCONFIG;
		cpio->compress = NULL;
		cpio->raw      = data;
		cpio->size     = size;
		cpio->headers  = NULL;
	}

	stream_level--;
	return;

bootconfig:
	hdr = (uint32_t *)(data - 8);

	uint32_t size_le;
	memcpy(&size_le, &hdr[0], sizeof(size_le));

	size = le32toh(size_le);
	data = ((void *)hdr) - size;

	if (size > arv->size)
		err(EXIT_FAILURE, "bootconfig size %d is greater than initrd size %ld", size, arv->size);

	/* Remove bootconfig from initramfs/initrd */
	arv->size = (unsigned long) (data - arv->addr);

	goto stream;
}

void
free_streams(struct list_tail *stream)
{
	struct list_tail *l = stream;
	while (l) {
		if (((struct stream *) l->data)->allocated) {
			free(((struct stream *) l->data)->addr);
		}
		l = l->next;
	}
	if (stream)
		list_free(stream);
}

void
free_cpios(struct list_tail *cpios)
{
	struct list_tail *l = cpios;
	while (l) {
		if (l->data != NULL)
			cpio_free(l->data);
		l = l->next;
	}
	list_free(cpios);
}
