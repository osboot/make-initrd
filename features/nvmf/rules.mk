# SPDX-License-Identifier: GPL-3.0-or-later
MODULES_LOAD += $(NVMF_MODULES)

PUT_FEATURE_FILES += \
	$(wildcard /etc/nvme/hostnqn) \
	$(wildcard /etc/nvme/hostid) \
	$(wildcard /etc/nvme/discovery.conf) \
	$(wildcard /etc/nvme/config.json)

PUT_FEATURE_PROGS += nvme
PUT_FEATURE_DIRS  += $(NVMF_DATADIR)
