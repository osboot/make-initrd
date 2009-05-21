ifndef _OPTIONAL_BOOTSPLASH_MK
_OPTIONAL_BOOTSPLASH_MK = 1

optional-bootsplash: pack
	@echo "Building bootsplash ..."
	@$(OPTIONAL_BOOTSPLASH)

install: optional-bootsplash

endif
