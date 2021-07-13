$(call feature-requires,depmod-image network)

SSHFS_DATADIR	= $(FEATURESDIR)/sshfsroot/data
SSHFS_PRELOAD	= af_packet fs-fuse
SSHFS_PROGS	= sshfs ssh ask-pass

SSHFS_KNOWN_HOSTS ?=
SSHFS_CONFIG ?=
SSHFS_KEY ?=
SSHFS_KEYS ?=

ifdef SSHFS_CONFIG
SSHFS_PKCS11_MODULE := $(shell $(FEATURESDIR)/sshfsroot/bin/get-pkcs11-module "$(SSHFS_CONFIG)")
endif

ifdef SSHFS_PKCS11_MODULE
SMART_CARD_PKCS11_MODULE = $(SSHFS_PKCS11_MODULE)
$(call feature-requires,smart-card)
endif
