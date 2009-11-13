$(call require,depmod-image)

autodetect: create
	@echo "Detecting modules..."
	@$(AUTODETECT_MODULES) $(AUTODETECT)

depmod-image: autodetect
