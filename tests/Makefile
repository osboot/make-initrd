.EXPORT_ALL_VARIABLES:

# qemu-based tests cannot be run in parallel.
.NOTPARALLEL:

TOPDIR  = $(realpath $(CURDIR)/..)
TESTDIR = $(TOPDIR)/.tests

VERBOSE ?= $(V)
Q = $(if $(VERBOSE),,@)

check_SUBDIRS := \
	test-luks \
	test-lukskey \
	test-lvm \
	test-partition \
	test-raid1 \
	test-degraded-raid1 \
	test-pipeline-iso-squash \
	test-btrfs-multi-device \
	test-btrfs-subvol

.PHONY: build $(check_SUBDIRS)

build:
	$(Q)$(MAKE) --no-print-directory -C "$(CURDIR)/$@" build;

$(check_SUBDIRS):
	$(Q)$(MAKE) --no-print-directory -C "$(CURDIR)/$@" check;

check: $(check_SUBDIRS)

clean:
	@echo "Cleanup tests ..."
	$(Q)rm -rf -- $(TESTDIR)
