TOPDIR  ?= $(realpath $(CURDIR)/../..)
TESTDIR ?= $(TOPDIR)/.tests

TESTCASE_DISK0 = $(TESTDIR)/data/test-partition-0.qcow2
TESTCASE_DISK1 = $(TESTDIR)/data/test-iso-squashfs.iso

TESTCASE_CMDLINE = testcase=$(TESTCASE) console=ttyS0 rootdelay=10 rootonly=1 panic=1 \
		   root=pipeline \
		   pipeline=waitdev,mountfs,mountfs,rootfs \
		   waitdev=/dev/cdrom \
		   mountfs=dev \
		   mountfs=image

TESTCASE_COMMAND = \
	$(QEMU_BIN) \
	    $(QEMU_ARGUMENTS) \
	    $(QEMU_RUNTIME_DRIVES) \
	    -kernel '$(TESTCASE_VMLINUZ)' \
	    -initrd '$(TESTCASE_INITRAMFS)' \
	    -append '$(TESTCASE_CMDLINE)'

include $(TOPDIR)/tests/Makefile.testsuite

TESTCASE_DRIVES =
TESTCASE_DRIVES += -drive 'media=disk,index=1,file=$(TESTCASE_DISK0)'
TESTCASE_DRIVES += -drive 'media=cdrom,index=2,file=$(TESTCASE_DISK1)'

QEMU_STAGE0_DRIVES  += $(TESTCASE_DRIVES)
QEMU_RUNTIME_DRIVES += $(TESTCASE_DRIVES)

$(TESTCASE_INITRAMFS):       $(TESTCASE_DISK0) $(TESTCASE_DISK1)
check: $(TESTCASE_INITRAMFS) $(TESTCASE_DISK0) $(TESTCASE_DISK1)
