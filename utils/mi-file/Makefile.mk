ifeq ($(HAVE_LIBELF),yes)
mi_file_DEST = $(dest_libexecdir)/mi-file
mi_file_SRCS = utils/mi-file/mi-file.c
mi_file_LIBS = $(shell pkg-config --libs libelf)

PROGS     += $(mi_file_DEST)
C_SOURCES += $(mi_file_SRCS)

else
$(warning Your system does not have libelf, disabling mi-file)
endif
