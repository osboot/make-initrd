clean: install
	@$(MSG) "Removing work directory ..."
	$Qrm -rf -- "$(ROOTDIR)" "$(GUESSDIR)" "$(WORKDIR)/initcpio"
	$Qrmdir -- "$(WORKDIR)"

genimage: clean
