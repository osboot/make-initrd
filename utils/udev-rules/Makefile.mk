# SPDX-License-Identifier: GPL-3.0-or-later

udev_rules_DEST = $(dest_sbindir)/udev-rules

udev_rules_SRCDIR = utils/udev-rules
udev_rules_GENDIR = $(GENDIR)/utils/udev-rules

udev_rules_SRCS = \
		$(udev_rules_GENDIR)/udev-rules-parser.c \
		$(udev_rules_GENDIR)/udev-rules-scanner.c \
		$(udev_rules_SRCDIR)/list_sort.c \
		$(udev_rules_SRCDIR)/udev-string.c \
		$(udev_rules_SRCDIR)/udev-goto-label.c \
		$(udev_rules_SRCDIR)/udev-rules.c

udev_rules_CFLAGS = -I$(udev_rules_SRCDIR) -I$(udev_rules_GENDIR)

PROGS += udev_rules

$(udev_rules_GENDIR)/udev-rules-parser.c: $(udev_rules_SRCDIR)/udev-rules-parser.y
	$(Q)mkdir -p -- $(dir $@)
	$(Q)touch -r \
	  $(udev_rules_SRCDIR)/udev-rules-scanner.l \
	  $(udev_rules_GENDIR)/udev-rules-scanner.h
	$(call quiet_cmd,YACC,$<,$(YACC)) -Wno-yacc \
	  --defines=$(udev_rules_GENDIR)/udev-rules-parser.h --output=$@ $<

$(udev_rules_GENDIR)/udev-rules-scanner.c: $(udev_rules_SRCDIR)/udev-rules-scanner.l $(udev_rules_GENDIR)/udev-rules-parser.c
	$(Q)mkdir -p -- $(dir $@)
	$(call quiet_cmd,LEX,$<,$(LEX)) \
	  --outfile=$@ --header-file=$(udev_rules_GENDIR)/udev-rules-scanner.h $<
