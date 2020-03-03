TOPDIR  ?= $(realpath $(CURDIR)/../..)
TESTDIR ?= $(TOPDIR)/.tests

TESTCASE_DISK0 = $(TESTDIR)/data/test-raid1-0.qcow2
TESTCASE_DISK1 = $(TESTDIR)/data/test-raid1-1.qcow2

TESTCASE_COMMAND = \
	$(QEMU_BIN) \
	    $(QEMU_ARGUMENTS) \
	    $(QEMU_RUNTIME_DRIVES) \
	    -kernel '$(TESTCASE_VMLINUZ)' \
	    -initrd '$(TESTCASE_INITRAMFS)' \
	    -append '$(TESTCASE_CMDLINE)'

include $(TOPDIR)/Makefile.testsuite

QEMU_STAGE0_DRIVES  += -drive 'media=disk,index=1,file=$(TESTCASE_DISK0)'
QEMU_STAGE0_DRIVES  += -drive 'media=disk,index=2,file=$(TESTCASE_DISK1)'
QEMU_RUNTIME_DRIVES += -drive 'media=disk,index=1,file=$(TESTCASE_DISK0)'

$(TESTCASE_INITRAMFS):       $(TESTCASE_DISK0) $(TESTCASE_DISK1)
check: $(TESTCASE_INITRAMFS) $(TESTCASE_DISK0) $(TESTCASE_DISK1)
