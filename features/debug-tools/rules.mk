ifeq "$(GDB_PROG)" ''
GDB_PROG = gdb
endif

ifneq '$(shell ldd "$(shell which $(GDB_PROG))" | grep libpython)' ''
GDB_PYTHON_PATHS := $(shell $(GDB_PROG) -q --ex 'python print(" ".join(sys.path))' --ex 'quit')
GDB_PYTHON_PATHS := $(filter-out %/dist-packages %/site-packages,$(GDB_PYTHON_PATHS))
endif

PUT_FEATURE_PROGS += $(DEBUG_TOOLS_PROGS) $(GDB_PROG)
PUT_FEATURE_FILES += $(DEBUG_TOOLS_FILES) $(wildcard $(GDB_PYTHON_PATHS)) $(GDB_SOURCES)
PUT_FEATURE_OPTIONAL_LIBS += $(DEBUG_TOOLS_OPTIONAL_LIBS)
