mdadm:
	@echo "Adding mdadm support ..."
	@$(TOOLSDIR)/put-file $(MDADM_FILES)

pack: mdadm
