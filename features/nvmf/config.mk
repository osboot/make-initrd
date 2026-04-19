# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,add-modules depmod-image network)

NVMF_MODULES = nvme \
	       nvme-core \
	       nvme-fabrics \
	       nvme-tcp

NVMF_OPTIONAL_MODULES = nvme-fc \
			nvme-rdma \
			lpfc \
			qla2xxx \
			8021q

NVMF_DATADIR = $(FEATURESDIR)/nvmf/data
