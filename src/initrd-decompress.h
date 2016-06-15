#ifndef INITRD_DECOMPRESS_H
#define INITRD_DECOMPRESS_H

typedef int (*decompress_fn)(unsigned char *inbuf, unsigned long ilen,
	unsigned char **outbuf, unsigned long *olen,
	unsigned long *inread);

decompress_fn decompress_method(const unsigned char *inbuf, unsigned long len, const char **name);

int gunzip(unsigned char *in, unsigned long in_size, unsigned char **o, unsigned long *olen, unsigned long *inread);
int bunzip2(unsigned char *in, unsigned long in_size, unsigned char **o, unsigned long *olen, unsigned long *inread);

#endif /* INITRD_DECOMPRESS_H */
