# SPDX-License-Identifier: GPL-3.0-or-later
MODULES_LOAD += $(ISCSI_MODULES)

PUT_FILES += $(wildcard /etc/iscsi/initiatorname.iscsi)
PUT_FEATURE_PROGS += iscsistart iscsi-iname
PUT_FEATURE_PROGS_WILDCARD += iscsi-gen-initiatorname
PUT_FEATURE_DIRS  += $(ISCSI_DATADIR)
