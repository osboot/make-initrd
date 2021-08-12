ifneq '$(shell ldd "$(shell which gdb)" | grep libpython)' ''
GDB_PYTHON_PATHS = $(shell gdb -q --ex 'python print(" ".join(sys.path))' --ex 'quit')
endif

PUT_FEATURE_PROGS += $(DEBUG_TOOLS_PROGS)
PUT_FEATURE_FILES += $(DEBUG_TOOLS_FILES) $(wildcard $(GDB_PYTHON_PATHS)) $(GDB_SOURCES)
