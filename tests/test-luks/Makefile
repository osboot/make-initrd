TOPDIR  ?= $(realpath $(CURDIR)/../..)
TESTDIR ?= $(TOPDIR)/.tests

TESTCASE_DISK0 = $(TESTDIR)/data/$(TESTCASE)-0.qcow2

TESTCASE_COMMAND = \
	$(QEMU_BIN) \
	    $(QEMU_ARGUMENTS) \
	    -kernel '$(TESTCASE_VMLINUZ)' \
	    -initrd '$(TESTCASE_INITRAMFS)' \
	    -append '$(TESTCASE_CMDLINE)'

include $(TOPDIR)/Makefile.testsuite

QEMU_ARGUMENTS += -drive 'media=disk,index=1,file=$(TESTCASE_DISK0)'

QEMU_FILTER_ARGUMENTS += \
	--expect='Enter passphrase for /dev/sda2:' \
	--process='echo qwerty >>$$QEMU_STDIN'

$(TESTCASE_INITRAMFS):       $(TESTCASE_DISK0)
check: $(TESTCASE_INITRAMFS) $(TESTCASE_DISK0)
