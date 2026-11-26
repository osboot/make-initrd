# SPDX-License-Identifier: GPL-3.0-or-later

ask_pass_DEST = $(dest_featuredir)/sshfsroot/bin/ask-pass
ask_pass_SRCS = $(features_srcdir)/sshfsroot/src/ask-pass/ask-pass.c
ask_pass_CFLAGS = $(runtime_CFLAGS)
ask_pass_LDFLAGS = $(runtime_LDFLAGS)

PROGS += ask_pass
