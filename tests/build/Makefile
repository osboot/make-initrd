TOPDIR  ?= $(realpath $(CURDIR)/../..)
TESTDIR ?= $(TOPDIR)/.tests

include $(TOPDIR)/Makefile.testsuite

#QEMU_STAGE0_ARGUMENTS += -hdb '$(CURDIR)/disk0.qcow2'

build: qemu-stage0
	@$(TRAVIS_START) "build.make-initrd"
	@echo "$(TESTCASE): Building make-initrd ... "
	$(TRAVIS_Q)echo "$(TESTCASE)" > $(TESTDIR)/cur;
	$(TRAVIS_Q)timeout --foreground -- 10m \
	$(QEMU_FILTER) --logfile=$(TESTDIR)/log.build -- \
	$(QEMU_BIN) \
	    $(QEMU_STAGE0_ARGUMENTS) \
	    -kernel '$(QEMU_STAGE0_VMLINUZ)' \
	    -initrd '$(QEMU_STAGE0_INITRAMFS)' \
	    -append '$(QEMU_STAGE0_CMDLINE)';
	@$(TRAVIS_STOP)

shell: qemu-stage0
	$(QEMU_BIN) \
	    $(QEMU_STAGE0_ARGUMENTS) \
	    -kernel '$(QEMU_STAGE0_VMLINUZ)' \
	    -initrd '$(QEMU_STAGE0_INITRAMFS)' \
	    -append '$(QEMU_STAGE0_CMDLINE)'
