ifeq ($(HAVE_LIBELF),yes)
mi_file_DEST = $(dest_execdir)/mi-file
mi_file_SRCS = utils/mi-file/mi-file.c
mi_file_LIBS = $(HAVE_LIBELF_LIBS)
mi_file_CFLAGS = $(HAVE_LIBELF_CFLAGS)

PROGS += mi_file

else
$(warning Your system does not have libelf, disabling mi-file)
endif
