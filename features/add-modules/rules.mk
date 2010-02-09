$(call require,depmod-image)

add-modules: create
	@if [ -n "$(MODULES_ADD)" ]; then \
		echo "Adding modules ..."; \
		[ -n "$(RESOLVE_MODALIAS)" ] && args= || args=--optional; \
		$(TOOLSDIR)/add-module $$args $(MODULES_ADD); \
	fi

preload-modules: create
	@if [ -n "$(MODULES_PRELOAD)" ]; then \
		echo "Adding modules (preload) ..."; \
		$(TOOLSDIR)/load-module --pre-udev $(MODULES_PRELOAD); \
	fi

load-modules: create
	@if [ -n "$(MODULES_LOAD)" ]; then \
		echo "Adding modules (postload) ..."; \
		$(TOOLSDIR)/load-module --post-udev $(MODULES_LOAD); \
	fi

depmod-image: add-modules preload-modules load-modules
