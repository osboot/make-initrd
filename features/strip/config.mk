# SPDX-License-Identifier: GPL-3.0-or-later

STRIP_IMAGE  = $(FEATURESDIR)/strip/bin/strip-image
STRIP       ?= strip
STRIP_FLAGS ?= -s --remove-section=*debug* --remove-section=.comment --remove-section=.note*
