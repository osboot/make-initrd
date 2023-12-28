spawn_shell_DEST = $(dest_data_bindir)/spawn-shell
spawn_shell_SRCS = datasrc/spawn-shell/spawn-shell.c

spawn_shell_CFLAGS = -D_GNU_SOURCE -Idatasrc/libinitramfs
spawn_shell_LIBS = -L$(dest_data_libdir) -linitramfs

PROGS += spawn_shell
