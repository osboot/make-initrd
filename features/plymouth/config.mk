PLYMOUTH_THEME  ?=
PLYMOUTH_MODULES = drivers/gpu/drm/
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_FILES   = \
	/bin/plymouth \
	/sbin/plymouthd \
	/etc/plymouth/plymouthd.conf \
	/usr/share/plymouth/themes/details/details.plymouth \
	/usr/share/plymouth/themes/text/text.plymouth \
	/usr/$(LIBNAME)/plymouth/details.so \
	/usr/$(LIBNAME)/plymouth/text.so \
	/usr/$(LIBNAME)/plymouth/renderers/drm.so \
	/usr/$(LIBNAME)/plymouth/renderers/frame-buffer.so \
	$(wildcard /usr/share/plymouth/themes/$(PLYMOUTH_THEME)/*)
