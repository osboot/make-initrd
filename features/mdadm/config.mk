DISABLE_FEATURES += raid

# mdadm < 3.3
MDAMD_RULES_OLD = \
	/lib/udev/rules.d/64-md-raid.rules

MDAMD_RULES = \
	/lib/udev/rules.d/63-md-raid-arrays.rules \
	/lib/udev/rules.d/64-md-raid-assembly.rules

MDADM_CONF ?= /etc/mdadm.conf

MDADM_FILES = \
	/sbin/mdadm \
	$(wildcard $(MDAMD_RULES) $(MDAMD_RULES_OLD)) \
	$(wildcard $(MDADM_CONF))

MDADM_DATADIR = \
	$(FEATURESDIR)/mdadm/data
