.PHONY: bootconfig

bootconfig: $(call if_feature,compress)
	@$(MSG) "Adding bootconfig ..."
	@$(FEATURESDIR)/bootconfig/bin/add-bootconfig

install: bootconfig
