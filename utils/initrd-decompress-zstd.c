#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <error.h>

#include <zstd.h>

#include "initrd-decompress.h"

static void *
xmalloc(size_t size)
{
	void *r = malloc(size);
	if (!r)
		error(EXIT_FAILURE, errno, "malloc: allocating %lu bytes", (unsigned long) size);
	return r;
}

int
unzstd(unsigned char *in, unsigned long in_size,
       unsigned char **out, unsigned long *out_size,
       unsigned long long *inread)
{
	void *buff_in, *buff_out;
	size_t to_read, pos = 0;

	ZSTD_DStream *dstream = ZSTD_createDStream();

	if (!dstream) {
		error(EXIT_SUCCESS, 0, "ERROR: %s: %d: ZSTD_createDStream",
		      __FILE__, __LINE__);
		return DECOMP_FAIL;
	}

	size_t result = ZSTD_initDStream(dstream);

	if (ZSTD_isError(result)) {
		error(EXIT_SUCCESS, 0, "ERROR: %s: %d: ZSTD_initDStream: %s",
		      __FILE__, __LINE__, ZSTD_getErrorName(result));
		ZSTD_freeDStream(dstream);
		return DECOMP_FAIL;
	}

	buff_in  = xmalloc(ZSTD_DStreamInSize());
	buff_out = xmalloc(ZSTD_DStreamOutSize());
	to_read  = result;

	while (pos < in_size) {
		size_t bytes_read = to_read;

		if ((in_size - pos) < to_read)
			bytes_read = in_size - pos;

		memcpy(buff_in, in + pos, bytes_read);
		pos += bytes_read;

		ZSTD_inBuffer input = { buff_in, bytes_read, 0 };

		while (input.pos < input.size) {
			ZSTD_outBuffer output = { buff_out, ZSTD_DStreamOutSize(), 0 };

			to_read = ZSTD_decompressStream(dstream, &output, &input);

			if (ZSTD_isError(to_read)) {
				error(EXIT_SUCCESS, 0, "ERROR: %s: %d: ZSTD_decompressStream: %s",
				      __FILE__, __LINE__, ZSTD_getErrorName(to_read));

				ZSTD_freeDStream(dstream);
				free(buff_in);
				free(buff_out);

				return DECOMP_FAIL;
			}

			*out = realloc(*out, *out_size + output.pos);
			memcpy(*out + *out_size, buff_out, output.pos);
			*out_size += output.pos;
		}
	}

	*inread = in_size;

	ZSTD_freeDStream(dstream);
	free(buff_in);
	free(buff_out);

	return DECOMP_OK;
}
