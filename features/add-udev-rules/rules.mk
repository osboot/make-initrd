PUT_UDEV_RULES += $(SYSTEM_UDEV_RULES)

.PHONY: put-udev-rules

put-udev-rules: create
	@$(VMSG) "Putting udev rules ..."
	@$(FEATURESDIR)/add-udev-rules/bin/put-rules

pack: put-udev-rules
