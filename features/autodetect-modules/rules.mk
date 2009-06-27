$(call require,depmod-image)

autodetect-modules:
	@echo "Detecting modules..."
	@$(AUTODETECT_MODULES)

depmod-image: autodetect-modules
