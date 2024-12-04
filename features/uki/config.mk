# SPDX-License-Identifier: GPL-3.0-or-later

$(call feature-requires,\
	$(call if-active-feature,btrfs mdadm lvm luks devmapper) \
	add-modules add-udev-rules \
	modules-blockdev \
	modules-crypto-user-api \
	modules-filesystem \
	modules-multiple-devices \
	modules-network \
	modules-nfs)

$(call feature-disables,compress)

UKI_PROGS = objdump objcopy

UKI_UEFI_STUB =
UKI_SBAT =
UKI_KERNEL =
UKI_CMDLINE =
UKI_SPLASH_IMAGE =

UKI_EFIDIR    ?= $(firstword $(wildcard /efi/EFI $(BOOTDIR)/efi/EFI $(BOOTDIR)))
UKI_IMAGEFILE ?= $(UKI_EFIDIR)/linux-$(KERNEL).efi
