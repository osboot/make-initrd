.PHONY: bootconfig

bootconfig: $(call if-feature,compress)
	@$(MSG) "Adding bootconfig ..."
	@$(FEATURESDIR)/bootconfig/bin/add-bootconfig

install: bootconfig
