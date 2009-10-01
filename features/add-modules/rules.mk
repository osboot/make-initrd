$(call require,depmod-image)

add-modules: create
	@if [ -n "$(MODULES_ADD)" ]; then \
		echo "Adding modules ..."; \
		$(ADD_MODULE) $(MODULES_ADD); \
	fi

preload-modules: create
	@if [ -n "$(MODULES_PRELOAD)" ]; then \
		echo "Adding modules (preload) ..."; \
		$(LOAD_MODULE) --pre-udev $(MODULES_PRELOAD); \
	fi

load-modules: create
	@if [ -n "$(MODULES_LOAD)" ]; then \
		echo "Adding modules (postload) ..."; \
		$(LOAD_MODULE) --post-udev $(MODULES_LOAD); \
	fi

depmod-image: add-modules preload-modules load-modules
