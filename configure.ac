AC_PREREQ([2.69])
AC_INIT([make-initrd],[2.60.0],[gladkov.alexey@gmail.com])
AC_CONFIG_HEADERS([config.h])

AC_PREFIX_DEFAULT([/usr])

AS_IF([test "$sysconfdir" = '${prefix}/etc'], [sysconfdir=/etc])
AC_SUBST([sysconfdir])

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_INSTALL
AC_PROG_MKDIR_P
AC_PROG_LN_S
AC_PROG_SED
AC_PROG_YACC
AC_PROG_LEX([noyywrap])

AC_ARG_ENABLE([lto],
	[AS_HELP_STRING([--enable-lto],
			[build C programs with link-time optimization])],
	[], [enable_lto=no])

AS_CASE([$enable_lto],
	[yes], [
		AC_MSG_CHECKING([whether $CC supports link-time optimization])
		saved_CFLAGS=$CFLAGS
		saved_LDFLAGS=$LDFLAGS
		CFLAGS="$CFLAGS -flto"
		LDFLAGS="$LDFLAGS -flto"
		AC_LINK_IFELSE(
			[AC_LANG_PROGRAM([], [return 0;])],
			[AC_MSG_RESULT([yes])],
			[CFLAGS=$saved_CFLAGS
			 LDFLAGS=$saved_LDFLAGS
			 AC_MSG_RESULT([no])
			 AC_MSG_FAILURE([$CC does not support link-time optimization])])
	],
	[no], [],
	[AC_MSG_ERROR([bad value '$enable_lto' for --enable-lto])])

AC_ARG_ENABLE([sanitizers],
	[AS_HELP_STRING([--enable-sanitizers],
			[build C programs with AddressSanitizer and UndefinedBehaviorSanitizer])],
	[], [enable_sanitizers=no])

AC_ARG_ENABLE([asan],
	[AS_HELP_STRING([--enable-asan],
			[build C programs with AddressSanitizer])],
	[], [enable_asan=$enable_sanitizers])

AS_CASE([$enable_asan],
	[yes], [
		AC_MSG_CHECKING([whether $CC supports AddressSanitizer])
		saved_CFLAGS=$CFLAGS
		saved_LDFLAGS=$LDFLAGS
		CFLAGS="$CFLAGS -fsanitize=address -fno-omit-frame-pointer"
		LDFLAGS="$LDFLAGS -fsanitize=address"
		AC_LINK_IFELSE(
			[AC_LANG_PROGRAM([], [return 0;])],
			[AC_MSG_RESULT([yes])],
			[CFLAGS=$saved_CFLAGS
			 LDFLAGS=$saved_LDFLAGS
			 AC_MSG_RESULT([no])
			 AC_MSG_FAILURE([$CC does not support AddressSanitizer])])
	],
	[no], [],
	[AC_MSG_ERROR([bad value '$enable_asan' for --enable-asan])])

AC_ARG_ENABLE([ubsan],
	[AS_HELP_STRING([--enable-ubsan],
			[build C programs with UndefinedBehaviorSanitizer])],
	[], [enable_ubsan=$enable_sanitizers])

AS_CASE([$enable_ubsan],
	[yes], [
		AC_MSG_CHECKING([whether $CC supports UndefinedBehaviorSanitizer])
		saved_CFLAGS=$CFLAGS
		saved_LDFLAGS=$LDFLAGS
		CFLAGS="$CFLAGS -fsanitize=undefined"
		LDFLAGS="$LDFLAGS -fsanitize=undefined"
		AC_LINK_IFELSE(
			[AC_LANG_PROGRAM([], [return 0;])],
			[AC_MSG_RESULT([yes])],
			[CFLAGS=$saved_CFLAGS
			 LDFLAGS=$saved_LDFLAGS
			 AC_MSG_RESULT([no])
			 AC_MSG_FAILURE([$CC does not support UndefinedBehaviorSanitizer])])
	],
	[no], [],
	[AC_MSG_ERROR([bad value '$enable_ubsan' for --enable-ubsan])])

AC_ARG_ENABLE([coverage],
	[AS_HELP_STRING([--enable-coverage],
			[build with code coverage instrumentation])],
	[], [enable_coverage=no])

AS_CASE([$enable_coverage],
	[yes], [
		AC_MSG_CHECKING([whether $CC supports code coverage instrumentation])
		saved_CFLAGS=$CFLAGS
		saved_LDFLAGS=$LDFLAGS
		CFLAGS="$CFLAGS -O0 --coverage"
		LDFLAGS="$LDFLAGS --coverage"
		AC_LINK_IFELSE(
			[AC_LANG_PROGRAM([], [return 0;])],
			[AC_MSG_RESULT([yes])],
			[CFLAGS=$saved_CFLAGS
			 LDFLAGS=$saved_LDFLAGS
			 AC_MSG_RESULT([no])
			 AC_MSG_FAILURE([$CC does not support code coverage instrumentation])])
	],
	[no], [],
	[AC_MSG_ERROR([bad value '$enable_coverage' for --enable-coverage])])

AC_SUBST([ENABLE_COVERAGE], [$enable_coverage])

m4_ifndef([PKG_PROG_PKG_CONFIG],
  [m4_fatal([Could not locate the pkg-config autoconf
    macros. These are usually located in /usr/share/aclocal/pkg.m4.
    If your macros are in a different location, try setting the
    environment variable AL_OPTS="-I/other/macro/dir" before running
    ./autogen.sh or autoreconf again. Make sure pkg-config is installed.])])
PKG_PROG_PKG_CONFIG

AC_SYS_LARGEFILE

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
		strpbrk strrchr strstr strtol strtoul strtoull uname \
		twalk_r tdestroy updwtmp])

saved_LIBS="$LIBS"
AC_SEARCH_LIBS([fts_close], [fts])
LIBS="$saved_LIBS"
case "$ac_cv_search_fts_close" in
	no) AC_MSG_FAILURE([failed to find fts_close]) ;;
	-l*) fts_LIBS="$ac_cv_search_fts_close" ;;
	*) fts_LIBS= ;;
esac
AC_SUBST([fts_LIBS])

AC_MSG_CHECKING([if in-tree build is required])
AC_ARG_ENABLE(local-build,
	[AS_HELP_STRING(--enable-local-build,
			[Build the project to run from a tree (developer mode)])],
	[MKLOCAL=$enableval],[MKLOCAL=no])
AC_MSG_RESULT([${MKLOCAL}])
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

AC_ARG_WITH([projectdir],
	[AS_HELP_STRING([--with-projectdir=DIR],
			[Directory for project data
			 (default: DATAROOTDIR/PACKAGE_NAME)])],
	[], [with_projectdir='${datarootdir}/${PACKAGE_NAME}'])
AC_SUBST([projectdir], [$with_projectdir])

AC_ARG_WITH([imagename],
	[AS_HELP_STRING([--with-imagename],
			[Initrd image name format (For example: 'initrd-$(KERNEL)$(IMAGE_SUFFIX).img' default: auto)])],
	[], [with_imagename='auto'])
AC_MSG_CHECKING([imagename format])
if test "$with_imagename" = "auto"; then
	with_imagename=""

	KERNEL="$(uname -r)"
	IMAGE_SUFFIX=

	for format in	'initrd-$(KERNEL)$(IMAGE_SUFFIX).img' \
			'initramfs-$(KERNEL)$(IMAGE_SUFFIX).img' \
			'initrd.img-$(KERNEL)$(IMAGE_SUFFIX)' \
			'initramfs.img-$(KERNEL)$(IMAGE_SUFFIX)' \
			'initrd-$(KERNEL)$(IMAGE_SUFFIX)' \
			'initramfs-$(KERNEL)$(IMAGE_SUFFIX)'; do
		eval "file=$bootdir/$(echo $format | tr '()' '{}')"
		if test -f "$file"; then
			test -z "$with_imagename" || AC_MSG_FAILURE(["Autodetection of image name format is ambiguous"])
			with_imagename="$format"
		fi
	done

	if test -z "$with_imagename"; then
		with_imagename='initrd-$(KERNEL)$(IMAGE_SUFFIX).img'
		AC_MSG_WARN(["Can't determinate image name format. Used default: $with_imagename"])
	fi
fi
AC_MSG_RESULT([$with_imagename])
with_imagename=$(echo "$with_imagename" | sed 's/\$/\$\$/g')
AC_SUBST([imagename], ["$with_imagename"])

m4_define([CHECK_SUBMODULE], [
	AS_IF([! test -f "$srcdir/$3"], [
		AS_IF([test -d "$srcdir/.git" || test -f "$srcdir/.git"], [
			AS_IF([command -v git >/dev/null 2>&1 && (cd "$srcdir" && git rev-parse --is-inside-work-tree >/dev/null 2>&1)], [
				AC_MSG_NOTICE([initializing submodule '$2'...])
				(cd "$srcdir" && git submodule update --init "$2")
			])
		])
		AS_IF([! test -f "$srcdir/$3"], [
			AS_IF([test -d "$srcdir/.git" || test -f "$srcdir/.git"], [
				AC_MSG_FAILURE([submodule '$1' is not initialized. Run: git submodule update --init])
			], [
				AC_MSG_FAILURE([directory '$srcdir/$2' is missing or incomplete])
			])
		])
	])
])

AC_ARG_WITH([busybox],
	[AS_HELP_STRING([--with-busybox],
			[Use builtin busybox (default: auto)])],
	[],
	[: m4_divert_text([DEFAULTS], [with_busybox=auto])])
AC_MSG_CHECKING([if builtin busybox is required])
if test "$with_busybox" != "no"; then
	if test "$with_busybox" = "auto"; then
		with_busybox=no
		if ! test -x "$with_runtimedir/bin/busybox"; then
			with_busybox=yes
		fi
	fi
	if test "$with_busybox" = "yes"; then
		CHECK_SUBMODULE([busybox], [external/busybox/upstream], [external/busybox/upstream/Makefile])
	fi
fi
AC_MSG_RESULT([$with_busybox])
AC_SUBST([USE_BUSYBOX], [$with_busybox])

AC_ARG_WITH([libshell],
	[AS_HELP_STRING([--with-libshell],
			[Use builtin libshell (default: auto)])],
	[],
	[: m4_divert_text([DEFAULTS], [with_libshell=auto])])
AC_MSG_CHECKING([if builtin libshell is required])
if test "$with_libshell" != "no"; then
	if test "$with_libshell" = "auto"; then
		with_libshell=no
		if ! test -f "/bin/shell-error" && ! test -f "$bindir/shell-error"; then
			with_libshell=yes
		fi
	fi
	if test "$with_libshell" = "yes"; then
		CHECK_SUBMODULE([libshell], [external/libshell/upstream], [external/libshell/upstream/Makefile])
	fi
fi
AC_MSG_RESULT([$with_libshell])
AC_SUBST([USE_LIBSHELL], [$with_libshell])

AC_ARG_WITH([udevd],
	[AS_HELP_STRING([--with-udevd=FILE],
			[Path to udevd daemon (default: auto)])],
	[], [])
if ! test -x "$with_udevd"; then
	AC_PATH_PROGS([with_udevd], [udevd eudevd systemd-udevd], [],
		[/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:${sbindir}:${bindir}:/lib/systemd:/usr/lib/systemd])
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
		[/sbin:/usr/sbin:/usr/local/sbin:/bin:/usr/bin:/usr/local/bin:${sbindir}:${bindir}:/lib/systemd:/usr/lib/systemd])
	if ! test -x "$with_udevadm"; then
		AC_MSG_FAILURE([required utility not found: udevadm])
	fi
fi
AC_SUBST([UDEVADM], [$with_udevadm])

# https://git.sr.ht/~sircmpwn/scdoc
AC_ARG_WITH([scdoc],
	[AS_HELP_STRING([--with-scdoc=FILE],
			[Path to scdoc utility (default: auto)])],
	[], [])
if ! test -x "$with_scdoc"; then
	AC_PATH_PROGS([with_scdoc], [scdoc])
fi
AC_SUBST([SCDOC], [$with_scdoc])

AC_PATH_PROGS([STRIP], [strip])
AC_SUBST([STRIP], [$STRIP])

# Checks for libraries.
AC_ARG_WITH([zlib],
	[AS_HELP_STRING([--with-zlib],
			[support zlib compression @<:@default=auto@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_zlib=yes])])

AS_IF([test "x$with_zlib" != xno],
	[PKG_CHECK_MODULES(HAVE_GZIP, zlib, [HAVE_GZIP=yes], [HAVE_GZIP=no])],
	[HAVE_GZIP=no])

AC_ARG_WITH([bzip2],
	[AS_HELP_STRING([--with-bzip2],
			[support bzip2 compression @<:@default=auto@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_bzip2=yes])])

AS_IF([test "x$with_bzip2" != xno],
	[PKG_CHECK_MODULES(HAVE_BZIP2, bzip2, [HAVE_BZIP2=yes], [HAVE_BZIP2=no])],
	[HAVE_BZIP2=no])

if test "x$HAVE_BZIP2" = xno; then
	AC_CHECK_LIB(bz2, BZ2_bzDecompressInit, [
		HAVE_BZIP2=yes
		HAVE_BZIP2_LIBS=-lbz2
		HAVE_BZIP2_CFLAGS=''
	], [HAVE_BZIP2=no])
fi

AC_ARG_WITH([lzma],
	[AS_HELP_STRING([--with-lzma],
			[support lzma compression @<:@default=auto@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_lzma=yes])])

AS_IF([test "x$with_lzma" != xno],
	[PKG_CHECK_MODULES(HAVE_LZMA, liblzma, [HAVE_LZMA=yes], [HAVE_LZMA=no])],
	[HAVE_LZMA=no])

AC_ARG_WITH([zstd],
	[AS_HELP_STRING([--with-zstd],
			[support zstd compression @<:@default=auto@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_zstd=yes])])

AS_IF([test "x$with_zstd" != xno],
	[PKG_CHECK_MODULES(HAVE_ZSTD, libzstd, [HAVE_ZSTD=yes], [HAVE_ZSTD=no])],
	[HAVE_ZSTD=no])

AC_ARG_WITH([libelf],
	[AS_HELP_STRING([--with-libelf],
			[use elf to detect file types @<:@default=auto@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_libelf=yes])])

AS_IF([test "x$with_libelf" != xno],
	[PKG_CHECK_MODULES(HAVE_LIBELF, libelf, [HAVE_LIBELF=yes], [HAVE_LIBELF=no])],
	[HAVE_LIBELF=no])

AC_ARG_WITH([json-c],
	[AS_HELP_STRING([--with-json-c],
			[use json-c to parse optional dependencies of systemd-like libraries @<:@default=auto@:>@])],
	[],
	[: m4_divert_text([DEFAULTS], [with_json_c=yes])])

AS_IF([test "x$with_json_c" != xno],
	[PKG_CHECK_MODULES(HAVE_LIBJSON_C, json-c, [HAVE_LIBJSON_C=yes], [HAVE_LIBJSON_C=no])],
	[HAVE_LIBJSON_C=no])

PKG_CHECK_MODULES(HAVE_LIBKMOD, libkmod, [HAVE_LIBKMOD=yes], [HAVE_LIBKMOD=no])

AS_IF([test "x$HAVE_LIBKMOD" != xyes],
	[AC_MSG_ERROR([libkmod was not found: https://git.kernel.org/pub/scm/utils/kernel/kmod/kmod.git])])

save_CFLAGS="$CFLAGS"
save_LIBS="$LIBS"
CFLAGS="$CFLAGS $HAVE_LIBKMOD_CFLAGS"
LIBS="$LIBS $HAVE_LIBKMOD_LIBS"
AC_CHECK_FUNCS([kmod_module_get_weakdeps])
CFLAGS="$save_CFLAGS"
LIBS="$save_LIBS"

AC_SUBST([HAVE_GZIP])
AC_SUBST([HAVE_BZIP2])
AC_SUBST([HAVE_LZMA])
AC_SUBST([HAVE_ZSTD])
AC_SUBST([HAVE_LIBELF])
AC_SUBST([HAVE_LIBJSON_C])
AC_SUBST([HAVE_LIBKMOD])
AC_SUBST([YACC])
AC_SUBST([LEX])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
