$(call feature-requires,depmod-image network)

SSHFS_DATADIR	= $(FEATURESDIR)/sshfsroot/data
SSHFS_PRELOAD	= af_packet fs-fuse
SSHFS_PROGS	= sshfs ssh ask-pass

SSHFS_KNOWN_HOSTS ?=
SSHFS_CONFIG ?=
SSHFS_KEY ?=
SSHFS_KEYS ?=
