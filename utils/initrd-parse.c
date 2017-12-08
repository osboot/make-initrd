#include <stdlib.h>
#include <errno.h>
#include <error.h>

#include "initrd-common.h"
#include "initrd-cpio.h"
#include "initrd-decompress.h"
#include "initrd-parse.h"

void
read_stream(const char *compress, struct stream *arv, struct result *res)
{
	struct list_tail *l;
	struct stream *a;
	struct cpio *cpio;
	const char *compress_name;
	decompress_fn decompress;

	unsigned long offset = 0;

	while(offset < arv->size) {
		decompress = decompress_method(arv->addr + offset, arv->size - offset, &compress_name);
		if (decompress) {
			unsigned char *unpack = NULL;
			unsigned long unpack_size = 0;
			unsigned long long readed = 0;

			//printf("Detected %s compressed data\n", compress_name);
			if (decompress(arv->addr + offset, arv->size - offset, &unpack, &unpack_size, &readed) != DECOMP_OK)
				error(EXIT_FAILURE, errno, "ERROR: %s: %d: decompressor failed", __FILE__, __LINE__);

			l = list_append(&res->streams, sizeof(struct stream));
			if (l == NULL)
				error(EXIT_FAILURE, errno, "ERROR: %s: %d: unable to add element to list", __FILE__, __LINE__);
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
			error(EXIT_FAILURE, errno, "ERROR: %s: %d: unable to add element to list", __FILE__, __LINE__);
		cpio = l->data;

		cpio->compress = compress;
		cpio->raw      = arv->addr + offset;
		cpio->size     = arv->size - offset;
		cpio->headers  = NULL;

		offset += read_cpio(cpio);
	}
}

void
free_streams(struct list_tail *stream)
{
	struct list_tail *l = stream;
	while (l) {
		if (((struct stream *)l->data)->allocated) {
			free(((struct stream *)l->data)->addr);
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

