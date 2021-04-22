$(call feature-requires,depmod-image)

PLYMOUTH_FONT  ?=
PLYMOUTH_THEME ?=

PLYMOUTH_FILES =
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_PROGS = plymouth plymouthd
