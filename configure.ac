AC_PREREQ([2.69])
AC_INIT([make-initrd],[2.10.0],[gladkov.alexey@gmail.com])
AC_CONFIG_SRCDIR([datasrc/getarg.c])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT([/usr])

sysconfdir=/etc
AC_SUBST([sysconfdir])

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_SED

m4_ifndef([PKG_PROG_PKG_CONFIG],
  [m4_fatal([Could not locate the pkg-config autoconf
    macros. These are usually located in /usr/share/aclocal/pkg.m4.
    If your macros are in a different location, try setting the
    environment variable AL_OPTS="-I/other/macro/dir" before running
    ./autogen.sh or autoreconf again. Make sure pkg-config is installed.])])
PKG_PROG_PKG_CONFIG

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([ \
		arpa/inet.h fcntl.h inttypes.h limits.h netinet/in.h stddef.h \
		stdint.h stdlib.h string.h sys/mount.h sys/param.h sys/socket.h \
		sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_HEADER_MAJOR
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_CHECK_FUNCS([ \
		clock_gettime gettimeofday inet_ntoa localtime_r memset munmap \
		regcomp setenv socket strchr strdup strerror strncasecmp strndup \
		strpbrk strrchr strstr strtol strtoul strtoull uname])

AC_ARG_ENABLE(local-build,
	[AS_HELP_STRING(--enable-local-build,
			[Build the project to run from a tree (developer mode)])],
	[MKLOCAL=$enableval],[MKLOCAL=no])
AC_SUBST([MKLOCAL])

AC_ARG_WITH([bootdir],
	[AS_HELP_STRING([--with-bootdir=DIR],
			[Directory for initramfs images (default: /boot)])],
	[], [with_bootdir='/boot'])
AC_SUBST([bootdir], [$with_bootdir])

AC_ARG_WITH([runtimedir],
	[AS_HELP_STRING([--with-runtimedir=DIR],
			[Directory for extra utilities for initramfs images
			 (default: /lib/initrd)])],
	[], [with_runtimedir='/lib/initrd'])
AC_SUBST([runtimedir], [$with_runtimedir])

AC_ARG_WITH([kbddir],
	[AS_HELP_STRING([--with-kbddir=DIR],
			[Directory where the console keymaps and fonts are located
			 (default: /lib/kbd)])],
	[], [with_kbddir='/lib/kbd'])
AC_SUBST([kbddir], [$with_kbddir])

AC_ARG_WITH([projectdir],
	[AS_HELP_STRING([--with-projectdir=DIR],
			[Directory for project data
			 (default: DATAROOTDIR/PACKAGE_NAME)])],
	[], [with_projectdir='${datarootdir}/${PACKAGE_NAME}'])
AC_SUBST([projectdir], [$with_projectdir])

AC_ARG_WITH([udevd],
	[AS_HELP_STRING([--with-udevd=FILE],
			[Path to udevd daemon (default: auto)])],
	[], [])
if ! test -x "$with_udevd"; then
	AC_PATH_PROGS([with_udevd], [udevd eudevd systemd-udevd], [],
		[/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:${sbindir}:${bindir}:${libdir}/systemd])
	if ! test -x "$with_udevd"; then
		AC_MSG_FAILURE([required utility not found: udevd])
	fi
fi
AC_SUBST([UDEVD], [$with_udevd])

AC_ARG_WITH([udevadm],
	[AS_HELP_STRING([--with-udevadm=FILE],
			[Path to udevadm utility (default: auto)])],
	[], [])
if ! test -x "$with_udevadm"; then
	AC_PATH_PROGS([with_udevadm], [udevadm eudevadm], [],
		[/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:${sbindir}:${bindir}:${libdir}/systemd])
	if ! test -x "$with_udevadm"; then
		AC_MSG_FAILURE([required utility not found: udevadm])
	fi
fi
AC_SUBST([UDEVADM], [$with_udevadm])

AC_ARG_WITH([help2man],
	[AS_HELP_STRING([--with-help2man=FILE],
			[Path to help2man utility (default: auto)])],
	[], [])
if ! test -x "$with_help2man"; then
	AC_PATH_PROGS([with_help2man], [help2man])
fi
AC_SUBST([HELP2MAN], [$with_help2man])

# Checks for libraries.
AC_ARG_WITH([zlib],
	[AS_HELP_STRING([--with-zlib],
			[support zlib compression @<:@default=yes@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_zlib=yes])])

AS_IF([test "x$with_zlib" != xno],
	[PKG_CHECK_MODULES(HAVE_GZIP, zlib, [HAVE_GZIP=yes], [HAVE_GZIP=no])],
	[HAVE_GZIP=no])

AC_ARG_WITH([bzip2],
	[AS_HELP_STRING([--with-bzip2],
			[support bzip2 compression @<:@default=yes@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_bzip2=yes])])

AS_IF([test "x$with_bzip2" != xno],
	[PKG_CHECK_MODULES(HAVE_BZIP2, bzip2, [HAVE_BZIP2=yes], [HAVE_BZIP2=no])],
	[HAVE_BZIP2=no])

AC_ARG_WITH([lzma],
	[AS_HELP_STRING([--with-lzma],
			[support lzma compression @<:@default=yes@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_lzma=yes])])

AS_IF([test "x$with_lzma" != xno],
	[PKG_CHECK_MODULES(HAVE_LZMA, liblzma, [HAVE_LZMA=yes], [HAVE_LZMA=no])],
	[HAVE_LZMA=no])

AC_ARG_WITH([zstd],
	[AS_HELP_STRING([--with-zstd],
			[support zstd compression @<:@default=yes@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_zstd=yes])])

AS_IF([test "x$with_zstd" != xno],
	[PKG_CHECK_MODULES(HAVE_ZSTD, libzstd, [HAVE_ZSTD=yes], [HAVE_ZSTD=no])],
	[HAVE_ZSTD=no])

AC_ARG_WITH([libelf],
	[AS_HELP_STRING([--with-libelf],
			[use elf to detect file types @<:@default=yes@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_libelf=yes])])

AS_IF([test "x$with_zstd" != xno],
	[PKG_CHECK_MODULES(HAVE_LIBELF, libelf, [HAVE_LIBELF=yes], [HAVE_LIBELF=no])],
	[HAVE_LIBELF=no])

PKG_CHECK_MODULES(HAVE_LIBKMOD, libkmod, [HAVE_LIBKMOD=yes], [HAVE_LIBKMOD=no])

AS_IF([test "x$HAVE_LIBKMOD" != xyes],
	[AC_MSG_ERROR([libkmod was not found: https://git.kernel.org/pub/scm/utils/kernel/kmod/kmod.git])])

AC_SUBST([HAVE_GZIP])
AC_SUBST([HAVE_BZIP2])
AC_SUBST([HAVE_LZMA])
AC_SUBST([HAVE_ZSTD])
AC_SUBST([HAVE_LIBELF])
AC_SUBST([HAVE_LIBKMOD])
AC_SUBST([MKLOCAL])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
