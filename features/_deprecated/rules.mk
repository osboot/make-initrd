ifeq "$(MAKECMDGOALS)" 'guess'
define show_deprecated =
$(if $(1),$(info WARNING: The "$(1)" feature is outdated and does nothing))
endef
$(call show_deprecated,$(patsubst $(FEATURESDIR)/%/rules.mk,%,$(lastword $(MAKEFILE_LIST))))
endif
