# SPDX-License-Identifier: GPL-3.0-or-later

# pata, sata, scsi, nvme
MODULES_TRY_ADD += /drivers/(block|ata|scsi|nvme|message/fusion)/

# IEEE 1394 (FireWire) support
MODULES_TRY_ADD += /drivers/firewire/

# MMC/SD/SDIO card support
MODULES_TRY_ADD += /(drivers/mmc|tifm_)

# NVDIMM (Non-Volatile Memory Device) Support
MODULES_TRY_ADD += /drivers/nvdimm/

# PCI controller drivers
MODULES_TRY_ADD += /drivers/pci/controller/

# Voltage and Current Regulator Support
MODULES_TRY_ADD += /drivers/regulator/

# SPI "Serial Peripheral Interface" support
MODULES_TRY_ADD += /drivers/spi/

# low-level drivers for certain pci/usb controllers
MODULES_TRY_ADD += /drivers/(mfd|clk|phy)/

# USB Storage Drivers
MODULES_TRY_ADD += /drivers/usb/storage/

# USB Host Controller Drivers
BLOCKDEV_PATTERN_SET = symbol:^(usb_get_hcd|usb_put_hcd|usb_add_hcd|usb_remove_hcd|usb_hcd_.*)
MODULES_PATTERN_SETS += BLOCKDEV_PATTERN_SET
