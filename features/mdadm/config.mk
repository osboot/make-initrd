DISABLE_FEATURES += raid

MDADM_FILES = \
	/sbin/mdadm \
	/lib/udev/rules.d/64-md-raid.rules \
	/etc/mdadm.conf
