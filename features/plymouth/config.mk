PLYMOUTH_DEFAULT_THEME := $(shell plymouth-set-default-theme 2>/dev/null ||:)

PLYMOUTH_THEME  ?= $(PLYMOUTH_DEFAULT_THEME)
PLYMOUTH_GET_MODULES = $(FEATURESDIR)/plymouth/bin/get-modules
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_FILES = \
	/bin/plymouth \
	/sbin/plymouthd \
	/etc/plymouth/plymouthd.conf \
	/usr/share/plymouth/plymouthd.defaults \
	/etc/altlinux-release \
	/var/lib/plymouth/boot-duration \
	/usr/share/plymouth/themes/details/details.plymouth \
	/usr/share/plymouth/themes/text/text.plymouth \
	/usr/$(LIBNAME)/plymouth/details.so \
	/usr/$(LIBNAME)/plymouth/text.so \
	/usr/$(LIBNAME)/plymouth/renderers/drm.so \
	/usr/$(LIBNAME)/plymouth/renderers/frame-buffer.so \
	$(shell $(FEATURESDIR)/plymouth/bin/get-theme-files $(PLYMOUTH_THEME))
