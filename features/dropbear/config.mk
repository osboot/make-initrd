# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,network syslog)

DROPBEAR_DATADIR = $(FEATURESDIR)/dropbear/data
DROPBEAR_PROGS   = dropbear dropbearkey dropbearconvert
