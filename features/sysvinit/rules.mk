sysvinit:
	@$(MSG) "Adding sysvinit support ..."
	@put-file "$(ROOTDIR)" $(SYSVINIT_FILES)
	@$(FEATURESDIR)/sysvinit/copy-sysvinit

sysvinit-sort-services:
	@$(MSG) "Sorting sysvinit services ..."
	@sort-lsb --rootdir="$(ROOTDIR)" --result=symlink

pack: sysvinit

pre-pack: sysvinit-sort-services
