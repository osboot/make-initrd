# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image add-udev-rules gpu-drm)

PLYMOUTH_FONT  ?=
PLYMOUTH_THEME ?=

PLYMOUTH_FILES =
PLYMOUTH_UDEV_RULES := $(wildcard $(FEATURESDIR)/plymouth/rules.d/*.rules)
PLYMOUTH_DATADIR = $(FEATURESDIR)/plymouth/data
PLYMOUTH_PROGS = plymouth plymouthd
PLYMOUTH_TEXTDOMAIN = plymouth
