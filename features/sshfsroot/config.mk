$(call feature-requires,depmod-image network)

SSHFS_DATADIR	= $(FEATURESDIR)/sshfsroot/data
SSHFS_PRELOAD	= af_packet fs-fuse
SSHFS_PROGS	= sshfs ssh ask-pass

SSHFS_KNOWN_HOSTS ?=
SSHFS_CONFIG ?=
SSHFS_KEY ?=
SSHFS_KEYS ?=

ifeq "$(call if-active-feature,sshfsroot)/$(call if-module-guessable,net)" "sshfsroot/net"

ifeq "$(GUESS_NET_IFACE)" ''
GUESS_MODULE += net
endif

ifeq "$(GUESS_NET_IFACE)" ''
GUESS_NET_IFACE = all
endif

endif
