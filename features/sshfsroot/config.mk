$(call feature-requires,depmod-image network)

SSHFS_DATADIR	= $(FEATURESDIR)/sshfsroot/data
SSHFS_PRELOAD	= af_packet fs-fuse
SSHFS_PROGS	= sshfs ssh ask-pass

SSHFS_KNOWN_HOSTS ?=
SSHFS_CONFIG ?=
SSHFS_KEY ?=
SSHFS_KEYS ?=

ifdef SSHFS_CONFIG
    SMART_CARD_PKCS11_MODULE ?= $(shell grep -w PKCS11Provider "$(SSHFS_CONFIG)" | tail -n1 | sed -e 's/[[:space:]]\+/ /g' -e 's/^ //' | cut -d" " -f 2)
endif

ifdef SMART_CARD_PKCS11_MODULE
    $(call feature-requires,smart-card)
endif
