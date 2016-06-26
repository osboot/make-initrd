#include <stdio.h>
#include <string.h>

#include "initrd-decompress.h"

struct compress_format {
	unsigned char magic[2];
	const char *name;
	decompress_fn decompressor;
};

static const struct compress_format compressed_formats[] = {
#ifdef HAVE_GZIP
	{ {0x1f, 0x8b}, "gzip",  gunzip  },
	{ {0x1f, 0x9e}, "gzip",  gunzip  },
#endif
#ifdef HAVE_BZIP2
	{ {0x42, 0x5a}, "bzip2", bunzip2 },
#endif
#ifdef HAVE_LZMA
	{ {0x5d, 0x00}, "lzma",  NULL    },
	{ {0xfd, 0x37}, "xz",    unlzma  },
#endif
	{ {0x89, 0x4c}, "lzo",   NULL    },
	{ {0x02, 0x21}, "lz4",   NULL    },
	{ {0, 0}, NULL, NULL }
};

decompress_fn decompress_method(const unsigned char *inbuf, unsigned long len, const char **name)
{
	const struct compress_format *cf;

	if (len < 2) {
		if (name)
			*name = NULL;
		return NULL;	/* Need at least this much... */
	}

	//printf("Compressed data magic: %#.2x %#.2x\n", inbuf[0], inbuf[1]);

	for (cf = compressed_formats; cf->name; cf++) {
		if (!memcmp(inbuf, cf->magic, 2))
			break;

	}
	if (name)
		*name = cf->name;

	if (cf->name && !cf->decompressor)
		printf("Decompression of '%s' is not supported\n", cf->name);

	return cf->decompressor;
}
