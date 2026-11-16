# SPDX-License-Identifier: GPL-3.0-or-later
PUT_FEATURE_PROGS          += $(filter-out none,$(CRASH_PROG)) cpio-stream
PUT_FEATURE_PROGS_WILDCARD += $(CRASH_DUMP_PROGS_PATTERNS)
PUT_FEATURE_FILES          += $(CRASH_EXTRA_PROGS)
PUT_FEATURE_DIRS           += $(CRASH_DATADIR)

PHONY += check-crash-size

check-crash-size: pack $(call if-active-feature,compress bootconfig)
	@$(FEATURESDIR)/crash/bin/check-crash-size "$(ROOTDIR)" "$(OUTFILE)"

install: check-crash-size

