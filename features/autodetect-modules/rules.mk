$(call require,depmod-image)

autodetect-modules: create
	@echo "Detecting modules..."
	@$(AUTODETECT_MODULES)

depmod-image: autodetect-modules
