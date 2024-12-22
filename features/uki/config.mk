# SPDX-License-Identifier: GPL-3.0-or-later

UKI_PROGS = objdump objcopy

UKI_FEATURES ?= \
	modules-blockdev \
	modules-crypto-user-api \
	modules-filesystem \
	modules-multiple-devices \
	modules-network \
	modules-nfs

UKI_UEFI_STUB ?=
UKI_SBAT ?=
UKI_VMLINUZ ?=
UKI_CMDLINE ?=
UKI_SPLASH_FILE ?=

#
# The files systemd-boot processes generally reside on the UEFI ESP which is
# usually mounted to /efi/, /boot/ or /boot/efi/ during OS runtime.
#
UKI_EFIDIR ?= $(firstword $(wildcard /efi/EFI $(BOOTDIR)/efi/EFI $(BOOTDIR)/EFI $(BOOTDIR)))

#
# Unified kernel boot entries following the Boot Loader Specification are
# read from /EFI/Linux/ on the ESP and the Extended Boot Loader partition.
#
UKI_IMAGEFILE ?= $(UKI_EFIDIR)/Linux/linux-$(KERNEL)$(IMAGE_SUFFIX).efi

FEATURES += $(if $(UKI),$(UKI_FEATURES))
