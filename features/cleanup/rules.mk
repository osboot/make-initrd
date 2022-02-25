.PHONY: clean

clean: install
	@$(VMSG) "Removing work directory ..."
	$Qrm -rf -- "$(ROOTDIR)" "$(GUESSDIR)" "$(WORKDIR)/initcpio" "$(WORKDIR)/root"
	$Qrmdir -- "$(WORKDIR)"

genimage: clean
