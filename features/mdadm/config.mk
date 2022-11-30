$(call feature-disables,raid)
$(call feature-requires,add-udev-rules)

# mdadm < 3.3
MDAMD_RULES_OLD = \
	*-md-raid.rules

MDAMD_RULES = \
	$(wildcard $(FEATURESDIR)/mdadm/rules.d/*.rules)

MDADM_CONF ?= $(SYSCONFDIR)/mdadm.conf

MDADM_PROGS = mdadm
MDADM_FILES = \
	$(wildcard $(MDADM_CONF))

MDADM_DATADIR = \
	$(FEATURESDIR)/mdadm/data
