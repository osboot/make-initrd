ifeq "$(call if-active-feature,sshfsroot)/$(call if-module-guessable,net)" "sshfsroot/net"

ifeq "$(GUESS_NET_IFACE)" ''
GUESS_MODULE += net
endif

ifeq "$(GUESS_NET_IFACE)" ''
GUESS_NET_IFACE = all
endif

endif
