$(call require,depmod-image)

rescue-modules: create
	@if [ -n "$(RESCUE_MODULES)" ]; then \
		$(MSG) "Adding rescue modules ..."; \
		$(TOOLSDIR)/add-rescue-modules $(RESCUE_MODULES); \
	fi

add-modules: create
	@if [ -n "$(MODULES_ADD)" ]; then \
		$(MSG) "Adding modules ..."; \
		[ -n "$(RESOLVE_MODALIAS)" ] && args= || args=--optional; \
		$(TOOLSDIR)/add-module $$args $(MODULES_ADD); \
	fi

preload-modules: create
	@if [ -n "$(MODULES_PRELOAD)" ]; then \
		$(MSG) "Adding modules (preload) ..."; \
		$(TOOLSDIR)/load-module --pre-udev $(MODULES_PRELOAD); \
	fi

load-modules: create
	@if [ -n "$(MODULES_LOAD)" ]; then \
		$(MSG) "Adding modules (postload) ..."; \
		$(TOOLSDIR)/load-module --post-udev $(MODULES_LOAD); \
	fi

add-deps: add-modules preload-modules load-modules rescue-modules
	@$(MSG) "Adding module dependencies ..."
	@$(TOOLSDIR)/add-module-deps

depmod-image: add-modules preload-modules load-modules rescue-modules add-deps
pack: rescue-modules
