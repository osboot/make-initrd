feature-configs := $(call wildcard-features,config.mk)
$(call include-features-once,$(feature-configs))

GUESSv1_SOURCES := $(sort $(shell $(TOOLSDIR)/get-guess-names))
GUESSv1_MODULES = $(if $(findstring all,$(AUTODETECT)),$(GUESSv1_SOURCES),$(AUTODETECT))
