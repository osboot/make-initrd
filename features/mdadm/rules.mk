mdadm:
	@echo "Adding mdadm support ..."
	@put-file "$(ROOTDIR)" $(MDADM_FILES)

pack: mdadm
