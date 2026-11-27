// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef INITRD_DECOMPRESS_H
#define INITRD_DECOMPRESS_H

#include "compiler.h"

#define DECOMP_OK 0
#define DECOMP_FAIL 1

typedef int (*decompress_fn)(unsigned char *inbuf, unsigned long ilen,
                             unsigned char **outbuf, unsigned long *olen,
                             unsigned long long *inread);

decompress_fn decompress_method(const unsigned char *inbuf, unsigned long len, const char **name)
	_rdonly_ptr_(1, 2) _access_(write_only, 3);

#ifdef HAVE_GZIP
int gunzip(unsigned char *in, unsigned long in_size, unsigned char **o, unsigned long *olen, unsigned long long *inread)
	_rdonly_ptr_(1, 2) _rdwr_ptr_(3) _rdwr_ptr_(4) _rdwr_ptr_(5);
#endif
#ifdef HAVE_BZIP2
int bunzip2(unsigned char *in, unsigned long in_size, unsigned char **o, unsigned long *olen, unsigned long long *inread)
	_rdonly_ptr_(1, 2) _rdwr_ptr_(3) _rdwr_ptr_(4) _rdwr_ptr_(5);
#endif
#ifdef HAVE_LZMA
int unlzma(unsigned char *in, unsigned long in_size, unsigned char **o, unsigned long *olen, unsigned long long *inread)
	_rdonly_ptr_(1, 2) _rdwr_ptr_(3) _rdwr_ptr_(4) _rdwr_ptr_(5);
#endif
#ifdef HAVE_ZSTD
int unzstd(unsigned char *in, unsigned long in_size, unsigned char **o, unsigned long *olen, unsigned long long *inread)
	_rdonly_ptr_(1, 2) _rdwr_ptr_(3) _rdwr_ptr_(4) _rdwr_ptr_(5);
#endif
#endif /* INITRD_DECOMPRESS_H */
