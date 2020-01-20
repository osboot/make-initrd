GPG_PROGRAM  ?= /usr/bin/gpg
LUKS_USE_GPG ?=

CRYPTSETUP_BIN		?= /usr/sbin/cryptsetup
LUKS_DATADIR		?= $(FEATURESDIR)/luks/data
LUKS_CIPHERS		?= aes
LUKS_BLOCKCIPHERS	?= cbc xts
LUKS_HASHES		?= sha256
LUKS_MODULES		 = dm-crypt af_packet $(LUKS_CIPHERS) $(LUKS_BLOCKCIPHERS) $(LUKS_HASHES)

ifeq "$(shell $(IS_KERNEL_VERSION) ge $(KERNEL) 5.4.0)" "true"
LUKS_BLOCKCIPHERS += essiv
endif
