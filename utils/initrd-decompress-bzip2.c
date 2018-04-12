#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include <bzlib.h>

#include "initrd-decompress.h"

#define CHUNK 0x4000

int
bunzip2(unsigned char *in, unsigned long in_size,
        unsigned char **out, unsigned long *out_size,
        unsigned long long *inread)
{
	int ret;
	unsigned long have, out_offset, total_in_hi32;
	bz_stream strm;
	char obuf[CHUNK];
	out_offset = 0;

	/* allocate inflate state */
	strm.bzalloc  = NULL;
	strm.bzfree   = NULL;
	strm.opaque   = NULL;
	strm.avail_in = 0;
	strm.next_in  = NULL;

	if ((ret = BZ2_bzDecompressInit(&strm, 0, 0)) != BZ_OK)
		return DECOMP_FAIL;

	strm.avail_in = (unsigned int) in_size;
	strm.next_in  = (char *) in;

	do {
		strm.avail_out = CHUNK;
		strm.next_out  = obuf;

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

	total_in_hi32 = strm.total_in_hi32;
	*inread += (total_in_hi32 << 32) + strm.total_in_lo32;

	/* clean up and return */
	BZ2_bzDecompressEnd(&strm);

	return ret == BZ_STREAM_END ? DECOMP_OK : DECOMP_FAIL;
}
