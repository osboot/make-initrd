ifeq "$(call if-recently-activated-feature,plymouth)" "plymouth"
GUESS_MODULES += locales
endif
