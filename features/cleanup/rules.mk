.PHONY: clean

clean: install
	@$(VMSG) "Removing work directory ..."
	$Qrm -rf -- "$(ROOTDIR)" "$(GUESSDIR)" "$(WORKDIR)/initcpio" "$(WORKDIR)/root"
	$Qprintf '%s\0' $(GARBAGE) | xargs -0 -r rm -rf --
	$Qrmdir -- "$(WORKDIR)"

genimage: clean
