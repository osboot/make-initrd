ifndef _COMPRESS_MK
_COMPRESS_MK = 1

compress: pack
	@echo "Compressing image ..."
	@$(COMPRESS_IMAGE)

install: compress

endif
