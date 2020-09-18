$(call feature-requires,depmod-image)

PLYMOUTH_DEFAULT_THEME := $(shell plymouth-set-default-theme 2>/dev/null ||:)

PLYMOUTH_THEME  ?= $(PLYMOUTH_DEFAULT_THEME)
PLYMOUTH_GET_MODULES = $(FEATURESDIR)/plymouth/bin/get-modules
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_PROGS = plymouth plymouthd
PLYMOUTH_FILES = \
	$(SYSCONFDIR)/plymouth/plymouthd.conf \
	$(wildcard $(SYSCONFDIR)/*-release) \
	/var/lib/plymouth/boot-duration \
	$(DATADIR)/plymouth/plymouthd.defaults \
	$(DATADIR)/plymouth/themes/details/details.plymouth \
	$(DATADIR)/plymouth/themes/text/text.plymouth \
	$(DATADIR)/fonts/ttf/dejavu/DejaVuSans.ttf \
	$(LIBDIR)/plymouth/details.so \
	$(LIBDIR)/plymouth/text.so \
	$(LIBDIR)/plymouth/label.so \
	$(LIBDIR)/plymouth/renderers/drm.so \
	$(LIBDIR)/plymouth/renderers/frame-buffer.so \
	$(shell env \
		"LIBDIR=$(LIBDIR)" \
		"DATADIR=$(DATADIR)" \
		$(FEATURESDIR)/plymouth/bin/get-theme-files $(PLYMOUTH_THEME))
