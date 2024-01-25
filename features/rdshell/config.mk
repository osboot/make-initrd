# SPDX-License-Identifier: GPL-3.0-or-later
$(call feature-requires,system-glibc)

$(call set-sysconfig,rdshell,RDSHELL_MODE,disable)

RDSHELL                         ?= shell
RDSHELL_PASSWORD                ?=
RDSHELL_COPY_PASSWORD_FROM_USER ?=
