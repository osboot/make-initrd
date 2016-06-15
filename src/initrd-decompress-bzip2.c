#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <bzlib.h>

#include "initrd-decompress.h"

int bunzip2(unsigned char *in, unsigned long in_size,
            unsigned char **out, unsigned long *out_size,
            unsigned long *inread)
{
	int ret;
	unsigned long have, out_offset;
	bz_stream strm;
	unsigned char obuf[CHUNK];
	out_offset = 0;

    /* allocate inflate state */
    strm.bzalloc  = NULL;
    strm.bzfree   = NULL;
    strm.opaque   = NULL;
    strm.avail_in = 0;
    strm.next_in  = NULL;

	if ((ret = BZ2_bzDecompressInit(&strm, 0, 0)) != BZ_OK)
		return ret;

	strm.avail_in = in_size;
	strm.next_in = in;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = obuf;

		ret = BZ2_bzDecompress(&strm);

		if (ret != BZ_OK && ret != BZ_STREAM_END)
			break;

		have = CHUNK - strm.avail_out;

		*out_size += have;
		*out = realloc(*out, sizeof(unsigned char *) * (*out_size));

		if (*out == NULL)
			error(EXIT_FAILURE, errno, "ERROR: %s: %d: realloc", __FILE__, __LINE__);

		memcpy(*out + out_offset, obuf, have);
		out_offset += have;
	} while (!strm.avail_out);

	*inread += (strm.total_in_hi32 << 32) + strm.total_in_lo32;

	/* clean up and return */
	BZ2_bzDecompressEnd(&strm);

	return ret == BZ_STREAM_END ? BZ_OK : BZ_DATA_ERROR;
}
