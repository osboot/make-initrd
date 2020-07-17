TOPDIR  ?= $(realpath $(CURDIR)/../..)
TESTDIR ?= $(TOPDIR)/.tests

TESTCASE_DISK0 = $(TESTDIR)/data/$(TESTCASE)-0.qcow2

TESTCASE_COMMAND = \
	$(QEMU_BIN) \
	    $(QEMU_ARGUMENTS) \
	    $(QEMU_RUNTIME_DRIVES) \
	    -kernel '$(TESTCASE_VMLINUZ)' \
	    -initrd '$(TESTCASE_INITRAMFS)' \
	    -append '$(TESTCASE_CMDLINE) rootflags=subvol=@'

include $(TOPDIR)/tests/Makefile.testsuite

TESTCASE_DRIVES = -drive 'media=disk,index=1,file=$(TESTCASE_DISK0)'

QEMU_STAGE0_DRIVES  += $(TESTCASE_DRIVES)
QEMU_RUNTIME_DRIVES += $(TESTCASE_DRIVES)

$(TESTCASE_INITRAMFS):       $(TESTCASE_DISK0)
check: $(TESTCASE_INITRAMFS) $(TESTCASE_DISK0)
