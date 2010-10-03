PLYMOUTH_THEME  ?= $(shell plymouth-set-default-theme)
PLYMOUTH_MODULES = drivers/gpu/drm/
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_PLUGIN  = $(shell $(FEATURESDIR)/plymouth/get-plugin $(PLYMOUTH_THEME))
PLYMOUTH_FILES   = \
	/bin/plymouth \
	/sbin/plymouthd \
	/etc/plymouth/plymouthd.conf \
	/usr/share/plymouth/plymouthd.defaults \
	/etc/altlinux-release \
	/usr/share/pixmaps/altlinux.png \
	/usr/share/plymouth/themes/details/details.plymouth \
	/usr/share/plymouth/themes/text/text.plymouth \
	/usr/$(LIBNAME)/plymouth/details.so \
	/usr/$(LIBNAME)/plymouth/text.so \
	/usr/$(LIBNAME)/plymouth/renderers/drm.so \
	/usr/$(LIBNAME)/plymouth/renderers/frame-buffer.so \
	/usr/$(LIBNAME)/plymouth/$(PLYMOUTH_PLUGIN).so \
	$(wildcard /usr/share/plymouth/themes/$(PLYMOUTH_THEME)/*)
