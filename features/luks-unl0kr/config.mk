$(call feature-requires, depmod-image add-udev-rules luks plymouth)

UNL0KR_PROGS = unl0kr

UNL0KR_FILES = \
	/etc/unl0kr.conf \
	/usr/share/libinput/ \
	$(wildcard /etc/libinput/)\
	/usr/share/X11/xkb/

UNL0KR_UDEV_RULES = *-input-id.rules
