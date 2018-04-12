#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include <lzma.h>

#include "initrd-decompress.h"

#define CHUNK 0x4000

int
unlzma(unsigned char *in, unsigned long in_size,
       unsigned char **out, unsigned long *out_size,
       unsigned long long *inread)
{
	unsigned long have, out_offset;
	lzma_ret ret;
	lzma_stream strm   = LZMA_STREAM_INIT;
	lzma_action action = LZMA_RUN;

	unsigned char obuf[CHUNK];
	out_offset = 0;

	if (lzma_stream_decoder(&strm, UINT64_MAX, 0) != LZMA_OK)
		return DECOMP_FAIL;

	strm.avail_in = in_size;
	strm.next_in  = in;

	do {
		strm.avail_out = CHUNK;
		strm.next_out  = obuf;

		ret = lzma_code(&strm, action);

		if (ret != LZMA_OK && ret != LZMA_STREAM_END)
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
	lzma_end(&strm);

	return ret == LZMA_STREAM_END ? DECOMP_OK : DECOMP_FAIL;
}
