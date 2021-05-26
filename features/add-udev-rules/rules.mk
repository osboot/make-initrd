.PHONY: put-udev-rules

put-udev-rules: create
	@$(MSG) "Putting udev rules ..."
	@$(FEATURESDIR)/add-udev-rules/bin/put-rules

pack: put-udev-rules
