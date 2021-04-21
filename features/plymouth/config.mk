$(call feature-requires,depmod-image)

PLYMOUTH_DEFAULT_THEME := $(shell plymouth-set-default-theme 2>/dev/null ||:)
PLYMOUTH_LIBDIR := $(shell $(FEATURESDIR)/plymouth/bin/get-libdir)
PLYMOUTH_FONT := $(shell fc-match -f '%{file}\n' 'DejaVuSans')

PLYMOUTH_THEME  ?= $(PLYMOUTH_DEFAULT_THEME)
PLYMOUTH_GET_MODULES = $(FEATURESDIR)/plymouth/bin/get-modules
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_PROGS = plymouth plymouthd
PLYMOUTH_FILES = \
	$(wildcard $(SYSCONFDIR)/plymouth/plymouthd.conf) \
	$(wildcard $(SYSCONFDIR)/*-release) \
	$(wildcard /var/lib/plymouth/boot-duration) \
	$(wildcard $(DATADIR)/plymouth/plymouthd.defaults) \
	$(DATADIR)/plymouth/themes/details/details.plymouth \
	$(DATADIR)/plymouth/themes/text/text.plymouth \
	$(PLYMOUTH_LIBDIR)/details.so \
	$(PLYMOUTH_LIBDIR)/text.so \
	$(PLYMOUTH_LIBDIR)/label.so \
	$(PLYMOUTH_LIBDIR)/renderers/drm.so \
	$(PLYMOUTH_LIBDIR)/renderers/frame-buffer.so \
	$(PLYMOUTH_FONT) \
	$(shell env \
		"LIBDIR=$(LIBDIR)" \
		"DATADIR=$(DATADIR)" \
		$(FEATURESDIR)/plymouth/bin/get-theme-files $(PLYMOUTH_THEME))
