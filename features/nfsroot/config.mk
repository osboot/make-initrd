# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image network)

NFS_DATADIR	= $(FEATURESDIR)/nfsroot/data
NFS_PRELOAD	= af_packet nfs
