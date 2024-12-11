# SPDX-License-Identifier: GPL-3.0-or-later

UKI_FEATURES = \
	modules-blockdev \
	modules-crypto-user-api \
	modules-filesystem \
	modules-multiple-devices \
	modules-network \
	modules-nfs

UKI_PROGS = objdump objcopy

UKI_UEFI_STUB ?=
UKI_SBAT ?=
UKI_VMLINUZ ?=
UKI_CMDLINE ?=
UKI_SPLASH_FILE ?=

UKI_EFIDIR    ?= $(firstword $(wildcard /efi/EFI $(BOOTDIR)/efi/EFI $(BOOTDIR)/EFI $(BOOTDIR)))
UKI_IMAGEFILE ?= $(UKI_EFIDIR)/Linux/linux-$(KERNEL)$(IMAGE_SUFFIX).efi

FEATURES += $(if $(UKI),$(UKI_FEATURES))
