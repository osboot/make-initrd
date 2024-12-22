# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image)

NETWORK_MODULES = ipv6 af_packet
NETWORK_DATADIR = $(FEATURESDIR)/network/data
NETWORK_PROGS   = ip udhcpc udhcpc6
