.PHONY: bootconfig

bootconfig: $(call if-feature,compress)
	@$(VMSG) "Adding bootconfig ..."
	@$(FEATURESDIR)/bootconfig/bin/add-bootconfig

install: bootconfig
