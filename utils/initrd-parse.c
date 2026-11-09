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

static void
find_pe_initrd(struct stream *arv)
{
	if (arv->size < 0x40)
		return;

	if (arv->addr[0] != 'M' || arv->addr[1] != 'Z')
		return;

	uint32_t pe_offset;
	memcpy(&pe_offset, arv->addr + 0x3c, sizeof(pe_offset));
	pe_offset = le32toh(pe_offset);

	if ((unsigned long)pe_offset + 24 > arv->size)
		return;

	if (memcmp(arv->addr + pe_offset, "PE\0\0", 4) != 0)
		return;

	uint16_t num_sections, opt_hdr_size;
	memcpy(&num_sections, arv->addr + pe_offset + 6, sizeof(num_sections));
	memcpy(&opt_hdr_size, arv->addr + pe_offset + 20, sizeof(opt_hdr_size));
	num_sections = le16toh(num_sections);
	opt_hdr_size = le16toh(opt_hdr_size);

	unsigned long sec_table = (unsigned long)pe_offset + 24 + opt_hdr_size;

	for (uint16_t i = 0; i < num_sections; i++) {
		unsigned long sec_offset = sec_table + (unsigned long)i * 40;
		if (sec_offset + 40 > arv->size)
			break;

		if (!memcmp(arv->addr + sec_offset, ".initrd\0", 8)) {
			uint32_t vsize, raw_size, raw_ptr;
			memcpy(&vsize, arv->addr + sec_offset + 8, sizeof(vsize));
			memcpy(&raw_size, arv->addr + sec_offset + 16, sizeof(raw_size));
			memcpy(&raw_ptr, arv->addr + sec_offset + 20, sizeof(raw_ptr));
			vsize = le32toh(vsize);
			raw_size = le32toh(raw_size);
			raw_ptr = le32toh(raw_ptr);

			if ((unsigned long)raw_ptr + raw_size > arv->size)
				errx(EXIT_FAILURE, "ERROR: .initrd section extends beyond file size");

			uint32_t sec_size = raw_size;
			if (vsize > 0 && vsize <= raw_size)
				sec_size = vsize;

			arv->addr += raw_ptr;
			arv->size = sec_size;
			return;
		}
	}

	errx(EXIT_FAILURE, "ERROR: PE image does not contain .initrd section");
}

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
		find_pe_initrd(arv);

		/*
		 * Check bootconfig first.
		 */
		if (arv->size >= BOOTCONFIG_MAGIC_LEN) {
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
