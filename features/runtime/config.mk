# SPDX-License-Identifier: GPL-3.0-or-later
USE_DEVICE_DAEMON ?= udev

ifneq ($(filter-out none,$(USE_DEVICE_DAEMON)),)
$(call feature-requires,$(if $(filter device-daemon-%,$(USE_DEVICE_DAEMON)),$(USE_DEVICE_DAEMON),device-daemon-$(USE_DEVICE_DAEMON)))
endif
