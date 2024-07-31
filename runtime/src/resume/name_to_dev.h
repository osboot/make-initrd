#ifndef __NAME_TO_DEV_H__
#define __NAME_TO_DEV_H__

#include <sys/sysmacros.h>

#define __makedev(__ma, __mi) \
	((((__ma) & 0xfff) << 8)|((__mi) & 0xff)|(((__mi) & 0xfff00) << 12))

#define	Root_RAM0	__makedev(1, 0)

/* These device numbers are only used internally */
#define Root_NFS	__makedev(0, 255)
#define Root_MTD	__makedev(0, 254)
#define Root_MULTI	__makedev(0, 253)

dev_t name_to_dev_t(const char *name);

#endif /* __NAME_TO_DEV_H__ */
