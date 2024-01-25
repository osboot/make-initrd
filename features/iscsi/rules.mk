# SPDX-License-Identifier: GPL-3.0-or-later
MODULES_LOAD += $(ISCSI_MODULES)

PUT_PROGS += iscsistart iscsi-gen-initiatorname iscsi-iname
PUT_FILES += $(wildcard /etc/iscsi/initiatorname.iscsi)
PUT_FEATURE_DIRS  += $(ISCSI_DATADIR)
