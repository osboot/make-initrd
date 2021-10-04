# Feature: sbc

Feature adds modules required for bootloading on single-board computers.
make-initrd on single board computers does not add required modules to
initramfs. These are various voltage regulators, i2c interfaces, etc.
Another reason is the ability to transfer the system from one board to
another, as well as from one interface of one board to another.
