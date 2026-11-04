// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _MOUNT_COMPAT_H
#define _MOUNT_COMPAT_H 1

#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <sys/fanotify.h>

#if defined(__has_include)
# if __has_include(<linux/types.h>)
#  include <linux/types.h>
# endif
#endif

#ifndef FAN_NONBLOCK
#define FAN_NONBLOCK 0x00000002
#endif

#ifndef FAN_REPORT_MNT
#define FAN_REPORT_MNT 0x00004000
#endif

#ifndef FAN_MARK_MNTNS
#define FAN_MARK_MNTNS 0x00000110
#endif

#ifndef FAN_MNT_ATTACH
#define FAN_MNT_ATTACH 0x01000000
#endif

#ifndef FAN_MNT_DETACH
#define FAN_MNT_DETACH 0x02000000
#endif

#ifndef FAN_EVENT_INFO_TYPE_FID
struct fanotify_event_info_header {
	uint8_t info_type;
	uint8_t pad;
	uint16_t len;
};
#endif

#ifndef FAN_EVENT_INFO_TYPE_MNT
#define FAN_EVENT_INFO_TYPE_MNT 7

struct fanotify_event_info_mnt {
	struct fanotify_event_info_header hdr;
	uint64_t mnt_id;
};
#endif

#ifndef __NR_statmount
# if defined(__x86_64__)
#  define __NR_statmount 457
# elif defined(__i386__)
#  define __NR_statmount 457
# elif defined(__aarch64__)
#  define __NR_statmount 457
# elif defined(__riscv)
#  define __NR_statmount 457
# endif
#endif

#ifndef __NR_listmount
# if defined(__x86_64__)
#  define __NR_listmount 458
# elif defined(__i386__)
#  define __NR_listmount 458
# elif defined(__aarch64__)
#  define __NR_listmount 458
# elif defined(__riscv)
#  define __NR_listmount 458
# endif
#endif

#ifndef __NR_pidfd_open
# if defined(__x86_64__)
#  define __NR_pidfd_open 434
# elif defined(__i386__)
#  define __NR_pidfd_open 434
# elif defined(__aarch64__)
#  define __NR_pidfd_open 434
# elif defined(__riscv)
#  define __NR_pidfd_open 434
# endif
#endif

#ifndef PIDFS_IOCTL_MAGIC
#define PIDFS_IOCTL_MAGIC 0xFF
#endif

#ifndef PIDFD_GET_MNT_NAMESPACE
#define PIDFD_GET_MNT_NAMESPACE _IO(PIDFS_IOCTL_MAGIC, 3)
#endif

#ifndef LSMT_ROOT
#define LSMT_ROOT 0xffffffffffffffffULL
#endif

#ifndef MNT_ID_REQ_SIZE_VER0
#define MNT_ID_REQ_SIZE_VER0 24
#endif

#ifndef STATMOUNT_SB_BASIC
#define STATMOUNT_SB_BASIC       0x00000001U
#endif
#ifndef STATMOUNT_MNT_BASIC
#define STATMOUNT_MNT_BASIC      0x00000002U
#endif
#ifndef STATMOUNT_PROPAGATE_FROM
#define STATMOUNT_PROPAGATE_FROM 0x00000004U
#endif
#ifndef STATMOUNT_MNT_ROOT
#define STATMOUNT_MNT_ROOT       0x00000008U
#endif
#ifndef STATMOUNT_MNT_POINT
#define STATMOUNT_MNT_POINT      0x00000010U
#endif
#ifndef STATMOUNT_FS_TYPE
#define STATMOUNT_FS_TYPE        0x00000020U
#endif
#ifndef STATMOUNT_MNT_NS_ID
#define STATMOUNT_MNT_NS_ID      0x00000040U
#endif
#ifndef STATMOUNT_MNT_OPTS
#define STATMOUNT_MNT_OPTS       0x00000080U
#endif
#ifndef STATMOUNT_FS_SUBTYPE
#define STATMOUNT_FS_SUBTYPE     0x00000100U
#endif
#ifndef STATMOUNT_SB_SOURCE
#define STATMOUNT_SB_SOURCE      0x00000200U
#endif

struct rd_mnt_id_req {
	uint32_t size;
	union {
		uint32_t mnt_ns_fd;
		uint32_t mnt_fd;
	};
	uint64_t mnt_id;
	uint64_t param;
	uint64_t mnt_ns_id;
};

struct rd_statmount {
	uint32_t size;
	uint32_t mnt_opts;
	uint64_t mask;
	uint32_t sb_dev_major;
	uint32_t sb_dev_minor;
	uint64_t sb_magic;
	uint32_t sb_flags;
	uint32_t fs_type;
	uint64_t mnt_id;
	uint64_t mnt_parent_id;
	uint32_t mnt_id_old;
	uint32_t mnt_parent_id_old;
	uint64_t mnt_attr;
	uint64_t mnt_propagation;
	uint64_t mnt_peer_group;
	uint64_t mnt_master;
	uint64_t propagate_from;
	uint32_t mnt_root;
	uint32_t mnt_point;
	uint64_t mnt_ns_id;
	uint32_t fs_subtype;
	uint32_t sb_source;
	uint32_t opt_num;
	uint32_t opt_array;
	uint32_t opt_sec_num;
	uint32_t opt_sec_array;
	uint64_t supported_mask;
	uint32_t mnt_uidmap_num;
	uint32_t mnt_uidmap;
	uint32_t mnt_gidmap_num;
	uint32_t mnt_gidmap;
	uint64_t __spare2[43];
	char str[];
};

static inline int sys_statmount(const struct rd_mnt_id_req *req, struct rd_statmount *sm, size_t bufsize, unsigned int flags)
{
#ifdef __NR_statmount
	return (int) syscall(__NR_statmount, req, sm, bufsize, flags);
#else
	(void) req;
	(void) sm;
	(void) bufsize;
	(void) flags;
	errno = ENOSYS;
	return -1;
#endif
}

static inline long sys_listmount(const struct rd_mnt_id_req *req, uint64_t *mnt_ids, size_t nr, unsigned int flags)
{
#ifdef __NR_listmount
	return syscall(__NR_listmount, req, mnt_ids, nr, flags);
#else
	(void) req;
	(void) mnt_ids;
	(void) nr;
	(void) flags;
	errno = ENOSYS;
	return -1;
#endif
}

static inline int sys_pidfd_open(pid_t pid, unsigned int flags)
{
#ifdef __NR_pidfd_open
	return (int) syscall(__NR_pidfd_open, pid, flags);
#else
	(void) pid;
	(void) flags;
	errno = ENOSYS;
	return -1;
#endif
}

#endif /* _MOUNT_COMPAT_H */
