$(call feature-requires,depmod-image devmapper modules-crypto-user-api system-glibc)

CRYPTSETUP_BIN		?= cryptsetup
LUKS_DATADIR		?= $(FEATURESDIR)/luks/data
LUKS_CIPHERS		?= aes
LUKS_BLOCKCIPHERS	?= cbc xts
LUKS_HASHES		?= sha256
LUKS_MODULES		 = dm-crypt af_packet $(LUKS_CIPHERS) $(LUKS_BLOCKCIPHERS) $(LUKS_HASHES)

ifdef $(call if_kernel_version_greater_or_equal,5.4.0)
LUKS_BLOCKCIPHERS += essiv
endif

ifeq "$(LUKS_CRYPTTAB)" ""
LUKS_CRYPTTAB = $(wildcard /etc/crypttab)
endif
