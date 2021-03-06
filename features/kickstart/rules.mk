KICKSTART_DIRS := $(shell $(FEATURESDIR)/kickstart/bin/get-ks-configs)

MODULES_ADD += $(LUKS_MODULES) drivers/md/raid

PUT_FEATURE_FILES += $(KICKSTART_FILES)
PUT_FEATURE_DIRS += $(KICKSTART_DATADIR) $(KICKSTART_DIRS)
PUT_FEATURE_PROGS += $(KICKSTART_PROGS)
PUT_FEATURE_PROGS_WILDCARD += $(KICKSTART_PROGS_PATTERNS)
