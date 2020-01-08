TOPDIR  ?= $(realpath $(CURDIR)/../..)
TESTDIR ?= $(TOPDIR)/.tests

TESTCASE_DISK0 = $(TESTDIR)/data/$(TESTCASE)-0.qcow2

TESTCASE_COMMAND = \
	$(CURDIR)/expect-password.sh \
	$(QEMU_BIN) \
	    $(QEMU_ARGUMENTS) \
	    -kernel '$(TESTCASE_VMLINUZ)' \
	    -initrd '$(TESTCASE_INITRAMFS)' \
	    -append '$(TESTCASE_CMDLINE)'

include $(TOPDIR)/Makefile.testsuite

QEMU_ARGUMENTS += -drive 'media=disk,index=1,file=$(TESTCASE_DISK0)'

$(TESTCASE_INITRAMFS):       $(TESTCASE_DISK0)
check: $(TESTCASE_INITRAMFS) $(TESTCASE_DISK0)
