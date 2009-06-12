$(call require,depmod-image)

add-modules: create
	@echo "Adding modules ..."
	@$(ADD_MODULE) $(MODULES_ADD)
	@$(LOAD_MODULE) $(MODULES_LOAD)

depmod-image: add-modules
