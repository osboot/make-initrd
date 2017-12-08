#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>
#include "zlib.h"

#include "initrd-decompress.h"

#define ENABLE_ZLIB_GZIP 32
#define windowBits 15
#define CHUNK 0x4000

int gunzip(unsigned char *in, unsigned long in_size,
           unsigned char **out, unsigned long *out_size,
           unsigned long long *inread)
{
	int ret;
	unsigned long have, out_offset;
	z_stream strm;
	unsigned char obuf[CHUNK];
	out_offset = 0;

    /* allocate inflate state */
    strm.zalloc   = Z_NULL;
    strm.zfree    = Z_NULL;
    strm.opaque   = Z_NULL;
    strm.avail_in = 0;
    strm.next_in  = Z_NULL;

	if ((ret = inflateInit2(&strm, windowBits|ENABLE_ZLIB_GZIP)) != Z_OK)
		return DECOMP_FAIL;

	strm.avail_in = (unsigned int) in_size;
	strm.next_in = in;

	do {
		strm.avail_out = CHUNK;
		strm.next_out = obuf;

		ret = inflate(&strm, Z_NO_FLUSH);

		if (ret != Z_OK && ret != Z_STREAM_END)
			break;

		have = CHUNK - strm.avail_out;

		*out_size += have;
		*out = realloc(*out, sizeof(unsigned char *) * (*out_size));

		if (*out == NULL)
			error(EXIT_FAILURE, errno, "ERROR: %s: %d: realloc", __FILE__, __LINE__);

		memcpy(*out + out_offset, obuf, have);
		out_offset += have;
	} while (!strm.avail_out);

	*inread += strm.total_in;

	/* clean up and return */
	inflateEnd(&strm);

	return ret == Z_STREAM_END ? DECOMP_OK : DECOMP_FAIL;
}
