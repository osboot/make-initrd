TOPDIR  ?= $(realpath $(CURDIR)/../..)
TESTDIR ?= $(TOPDIR)/.tests

TESTCASE_DISK0 = $(TESTDIR)/data/$(TESTCASE)-0.qcow2
TESTCASE_DISK1 = $(TESTDIR)/data/$(TESTCASE)-1.qcow2

TESTCASE_COMMAND = \
	$(QEMU_BIN) \
	    $(QEMU_ARGUMENTS) \
	    -kernel '$(TESTCASE_VMLINUZ)' \
	    -initrd '$(TESTCASE_INITRAMFS)' \
	    -append '$(TESTCASE_CMDLINE) luks-key=secret:LABEL=ROOTKEY'

include $(TOPDIR)/Makefile.testsuite

QEMU_ARGUMENTS += -drive 'media=disk,index=1,file=$(TESTCASE_DISK0)'
QEMU_ARGUMENTS += -drive 'media=disk,index=2,file=$(TESTCASE_DISK1)'

$(TESTCASE_INITRAMFS):       $(TESTCASE_DISK0) $(TESTCASE_DISK1)
check: $(TESTCASE_INITRAMFS) $(TESTCASE_DISK0) $(TESTCASE_DISK1)
