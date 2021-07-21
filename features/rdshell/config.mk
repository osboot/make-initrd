$(call feature-requires,system-glibc)

RDSHELL                         ?= shell
RDSHELL_PASSWORD                ?=
RDSHELL_COPY_PASSWORD_FROM_USER ?=

GUESSv1_RDSHELL_SOURCE := $(current-feature-name)
GUESSv1_RDSHELL_RULE = VAR{FEATURES}+=rdshell
