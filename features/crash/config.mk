# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,device-daemon-mdev)
$(call feature-disables,plymouth ucode gpu-drm device-daemon-udev add-udev-rules)

CRASH_ENABLE_LIVE ?= 1
ifeq ($(CRASH_ENABLE_LIVE),0)
CRASH_PROG :=
else
CRASH_PROG ?= crash
endif

CRASH_DATADIR = $(FEATURESDIR)/crash/data
CRASH_DUMP_PROGS_PATTERNS ?= \
	*/makedumpfile \
	*/vmcore-dmesg \
	*/findfs

CRASH_EXTRA_PROGS ?= $(wildcard $(shell which makedumpfile vmcore-dmesg findfs 2>/dev/null))
