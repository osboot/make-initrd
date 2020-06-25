$(call require,depmod-image)

ADD_MODULES_PATH = $(FEATURESDIR)/add-modules/bin

rescue-modules: create
	@export PATH="$(ADD_MODULES_PATH):$$PATH"; \
	if [ -n "$(RESCUE_MODULES)" ]; then \
		$(MSG) "Adding rescue modules ..."; \
		add-rescue-modules $(RESCUE_MODULES); \
	fi

add-modules: create
	@export PATH="$(ADD_MODULES_PATH):$$PATH"; \
	if [ -n "$(MODULES_ADD)" ]; then \
		$(MSG) "Adding modules ..."; \
		[ -n "$(RESOLVE_MODALIAS)" ] && args= || args=--optional; \
		add-module $$args $(MODULES_ADD); \
	fi
	@export PATH="$(ADD_MODULES_PATH):$$PATH"; \
	if [ -n "$(MODULES_TRY_ADD)" ]; then \
		$(MSG) "Adding optional modules ..."; \
		add-module --optional $(MODULES_TRY_ADD); \
	fi
	@export PATH="$(ADD_MODULES_PATH):$$PATH"; \
	if [ -n "$(MODULES_PATTERN_SETS)" ]; then \
		$(MSG) "Adding modules by pattern sets ..."; \
		add-module-pattern; \
	fi

preload-modules: create
	@export PATH="$(ADD_MODULES_PATH):$$PATH"; \
	if [ -n "$(MODULES_PRELOAD)" ]; then \
		$(MSG) "Adding modules (preload) ..."; \
		load-module --pre-udev $(MODULES_PRELOAD); \
	fi

load-modules: create
	@export PATH="$(ADD_MODULES_PATH):$$PATH"; \
	if [ -n "$(MODULES_LOAD)" ]; then \
		$(MSG) "Adding modules (postload) ..."; \
		load-module --post-udev $(MODULES_LOAD); \
	fi

add-deps: add-modules preload-modules load-modules rescue-modules
	@export PATH="$(ADD_MODULES_PATH):$$PATH"; \
	$(MSG) "Adding module dependencies ..."; \
	add-module-deps;

depmod-image: add-modules preload-modules load-modules rescue-modules add-deps
pack: rescue-modules
