.PHONY: compress

compress: pack
	@$(VMSG) "Compressing image ..."
	@$(COMPRESS_IMAGE)

install: compress
