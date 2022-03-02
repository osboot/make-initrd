$(call feature-requires,depmod-image gpu-drm)

PLYMOUTH_FONT  ?=
PLYMOUTH_THEME ?=

PLYMOUTH_FILES =
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_PROGS = plymouth plymouthd
PLYMOUTH_TEXTDOMAIN = plymouth
