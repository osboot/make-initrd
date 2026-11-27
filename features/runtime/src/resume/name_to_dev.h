#ifndef MKRD_NAME_TO_DEV_H
#define MKRD_NAME_TO_DEV_H

#include <sys/sysmacros.h>

#define RD_MAKEDEV(ma, mi) \
	((((ma) & 0xfff) << 8)|((mi) & 0xff)|(((mi) & 0xfff00) << 12))

#define	Root_RAM0	RD_MAKEDEV(1, 0)

/* These device numbers are only used internally */
#define Root_NFS	RD_MAKEDEV(0, 255)
#define Root_MTD	RD_MAKEDEV(0, 254)
#define Root_MULTI	RD_MAKEDEV(0, 253)

dev_t name_to_dev_t(const char *name);

#endif /* MKRD_NAME_TO_DEV_H */
