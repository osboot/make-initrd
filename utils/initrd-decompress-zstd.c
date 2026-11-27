// SPDX-License-Identifier: GPL-3.0-or-later

#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <limits.h>

#include <zstd.h>

#include "cleanup.h"
#include "initrd-decompress.h"

DEFINE_FREE(zstd_dstream, ZSTD_DStream *, if (_t) ZSTD_freeDStream(_t))

static void *
xmalloc(size_t size)
{
	void *r = malloc(size);
	if (!r)
		err(EXIT_FAILURE, "malloc: allocating %lu bytes", (unsigned long) size);
	return r;
}

int
unzstd(unsigned char *in, unsigned long in_size,
       unsigned char **out, unsigned long *out_size,
       unsigned long long *inread)
{
	void *buff_in _free_(free) = NULL, *buff_out _free_(free) = NULL;
	size_t to_read, pos = 0;

	ZSTD_DStream *dstream _free_(zstd_dstream) = ZSTD_createDStream();
	if (!dstream) {
		warnx("ERROR: %s: %d: ZSTD_createDStream", __FILE__, __LINE__);
		return DECOMP_FAIL;
	}

	to_read  = ZSTD_DStreamInSize();
	buff_in  = xmalloc(to_read);
	buff_out = xmalloc(ZSTD_DStreamOutSize());

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
				warnx("ERROR: %s: %d: ZSTD_decompressStream: %s",
				      __FILE__, __LINE__, ZSTD_getErrorName(to_read));
				return DECOMP_FAIL;
			}

			if (output.pos > (size_t) ULONG_MAX ||
			    *out_size > ULONG_MAX - (unsigned long) output.pos) {
				warnx("ERROR: %s: %d: output too large", __FILE__, __LINE__);
				return DECOMP_FAIL;
			}

			unsigned long new_size = *out_size + (unsigned long) output.pos;
			void *new_out = realloc(*out, new_size);

			if (!new_out) {
				warn("ERROR: %s: %d: realloc", __FILE__, __LINE__);
				return DECOMP_FAIL;
			}

			*out = new_out;
			memcpy(*out + *out_size, buff_out, output.pos);
			*out_size = new_size;
		}
	}

	*inread = in_size;
	return DECOMP_OK;
}
