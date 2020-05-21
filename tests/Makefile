.EXPORT_ALL_VARIABLES:

# qemu-based tests cannot be run in parallel.
.NOTPARALLEL:

TOPDIR  ?= $(realpath $(CURDIR)/..)
TESTDIR ?= $(TOPDIR)/.tests

check_SUBDIRS := \
	test-luks \
	test-lukskey \
	test-lvm \
	test-partition \
	test-raid1 \
	test-degraded-raid1 \
	test-pipeline-iso-squash

.PHONY: build $(check_SUBDIRS)

include $(TOPDIR)/Makefile.common

build:
	$(Q)$(MAKE) --no-print-directory -C "$(CURDIR)/$@" build;

$(check_SUBDIRS):
	$(Q)$(MAKE) --no-print-directory -C "$(CURDIR)/$@" check;

check: $(check_SUBDIRS)

check-clean:
	@echo "Cleanup tests ..."
	$(Q)rm -rf -- $(TESTDIR)
