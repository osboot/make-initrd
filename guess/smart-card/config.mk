ifeq "$(call if-recently-activated-feature,sshfsroot)" "sshfsroot"
GUESS_MODULES += smart-card
endif
