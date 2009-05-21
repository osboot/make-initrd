ifndef _OPTIONAL_BOOTSPLASH_MK
_OPTIONAL_BOOTSPLASH_MK = 1

optional-bootsplash: create
	@echo "Building bootsplash ..."
	@$(OPTIONAL_BOOTSPLASH)

pack: optional-bootsplash

endif
