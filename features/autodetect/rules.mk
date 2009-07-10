$(call require,depmod-image)

autodetect: create
	@echo "Detecting modules..."
	@for m in $(AUTODETECT); do \
		$(AUTODETECT_MODULES) $$m; \
	done

depmod-image: autodetect
