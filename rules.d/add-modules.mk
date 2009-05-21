ifndef _ADD_MODULES_MK
_ADD_MODULES_MK = 1

add-modules: create
	@echo "Adding modules ..."
	@$(ADD_MODULE) $(MODULES_ADD)
	@$(LOAD_MODULE) $(MODULES_LOAD)

depmod-image: add-modules

include $(RULESDIR)/depmod-image.mk
endif
