$(call feature-requires,depmod-image network)

SSHFS_DATADIR	= $(FEATURESDIR)/sshfsroot/data
SSHFS_PRELOAD	= af_packet fs-fuse
SSHFS_PROGS	= sshfs ssh ask-pass

SSHFS_KNOWN_HOSTS ?=
SSHFS_CONFIG ?=
SSHFS_KEY ?=
SSHFS_KEYS ?=

ifdef SSHFS_CONFIG
SSHFS_PKCS11_MODULES := $(shell $(FEATURESDIR)/sshfsroot/bin/get-pkcs11-module "$(SSHFS_CONFIG)")
ifneq "$(words $(sort $(SSHFS_PKCS11_MODULES)))" "1"
$(error Different PKCS11 providers are not supported for sshfsroot)
endif
endif

ifdef SSHFS_PKCS11_MODULES
SMART_CARD_PKCS11_MODULE = $(sort $(SSHFS_PKCS11_MODULES))
$(call feature-requires,smart-card)
endif
