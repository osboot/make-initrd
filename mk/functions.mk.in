ifeq "$(__included-functions-mk)" ''
__included-functions-mk := 1

null  :=
space := $(null) $(null)

# $(call basedirname,<filename>)
# Return directory name of <filename>.
define basedirname
$(if ($1),$(foreach n,$(1),$(notdir $(abspath $(dir $(n))))))
endef

# $(call current-feature-name)
define current-feature-name
$(strip $(call basedirname,$(lastword $(MAKEFILE_LIST))))
endef

define __feature-disables
$(if $(1),$(eval FEATURE-DISABLES-$(1) += $(2)))
endef

# $(call feature-disables,<name> <name1> ...)
# Set a list of conflicting features of the current feature.
define feature-disables
$(if $(1),\
	$(if $(filter %/config.mk,$(lastword $(MAKEFILE_LIST))),\
		$(call __feature-disables,$(strip $(call current-feature-name)),$(1))))
endef

define __feature-requires
$(if $(1),$(foreach n,$(2),$(eval FEATURE-REQUIRED-$(n) += $(1))))
$(if $(1),$(eval FEATURE-REQUIRES-$(1) += $(2)))
endef

# $(call feature-requires,<name> <name1> ...)
# Set a list of dependencies for the current feature.
define feature-requires
$(if $(1),\
	$(if $(filter %/config.mk,$(lastword $(MAKEFILE_LIST))),\
		$(call __feature-requires,$(strip $(call current-feature-name)),$(1))))
endef

# This is helper to filter a disabled files.
# $(call wildcard-features,<filename>)
# Return a space-separated list of full <filename> paths for each feature.
define wildcard-features
$(if $(1),\
	$(filter-out \
		$(foreach exl,$(get-all-disable-features),\
			$(realpath $(exl:%=$(FEATURESDIR)/%/$(1)))), \
		$(realpath $(wildcard $(FEATURESDIR)/*/$(1)))))
endef

define show-feature-dependency
$(foreach n,$(if $(1),$(1),$(sort $(foreach f,$(wildcard $(FEATURESDIR)/*/rules.mk),$(call basedirname,$(f))))),\
	$(info Feature $(n)) \
	$(info $(space) requires : $(sort $(FEATURE-REQUIRES-$(n)))) \
	$(info $(space) disables : $(sort $(FEATURE-DISABLES-$(n)))) \
	$(info $(space) required : $(sort $(FEATURE-REQUIRED-$(n)))) \
	$(info $(null)))
endef

define get-all-features
$(sort $(filter-out ,$(call expand-features,FEATURE-REQUIRES,$(FEATURES))))
endef

define get-all-disable-features
$(sort $(filter-out ,$(DISABLE_FEATURES) $(call expand-disabled-features,$(get-all-features))))
endef

define get-all-active-features
$(sort $(filter-out \
        $(call expand-list,FEATURE-REQUIRED,$(get-all-disable-features)),\
        $(get-all-features)))
endef

# The following functions should be available in both config.mk and rules.mk

# $(call if-success,<command>,<then>,<else>)
# Return <then> if <command> exits with 0, <else> otherwise.
define if-success
$(if $(1),$(shell { $(1); } >/dev/null 2>&1 && echo "$(2)" || echo "$(3)"))
endef

# $(call success,<command>)
# Return y if <command> exits with 0, n otherwise
define success
$(if $(1),$(call if-success,$(1),y,n))
endef

# $(call failure,<command>)
# Return n if <command> exits with 0, y otherwise
define failure
$(if $(1),$(call if-success,$(1),n,y))
endef

define if_kernel_version_greater_or_equal
$(if $(1),$(call if-success,$(TOOLSDIR)/kernel-compare "ge" "$(KERNEL)" "$(1)",PROJECT,))
endef

define if_kernel_version_greater
$(if $(1),$(call if-success,$(TOOLSDIR)/kernel-compare "gt" "$(KERNEL)" "$(1)",PROJECT,))
endef

define if_kernel_version_less_or_equal
$(if $(1),$(call if-success,$(TOOLSDIR)/kernel-compare "le" "$(KERNEL)" "$(1)",PROJECT,))
endef

define if_kernel_version_less
$(if $(1),$(call if-success,$(TOOLSDIR)/kernel-compare "lt" "$(KERNEL)" "$(1)",PROJECT,))
endef

define if_kernel_version_equal
$(if $(1),$(call if-success,$(TOOLSDIR)/kernel-compare "eq" "$(KERNEL)" "$(1)",PROJECT,))
endef

USED_FEATURES =

# Features include helper: skip a feature which has already loaded
# and skip globally disabled features
define include-feature
$(if $(2),$(eval USED_FEATURES += $(1)))
$(if $(2),$(eval include $(2)))
endef

define get-disabled-features
$(foreach exl,$(DISABLE_FEATURES),$(abspath $(exl:%=$(FEATURESDIR)/%/rules.mk)))
endef

define require
$(foreach n,$(1), \
	$(foreach inc,$(filter-out \
		$(MAKEFILE_LIST) $(call get-disabled-features), \
		$(abspath $(n:%=$(FEATURESDIR)/%/rules.mk))), \
		$(call include-feature,$(n),$(inc))))
endef

# $(call if-active-feature,<name> [<name1> ...])
define if-active-feature
$(if $(1),$(filter $(1),$(get-all-active-features)))
endef

# $(call if-recently-activated-feature,<name> [<name1> ...])
define if-recently-activated-feature
$(if $(1),$(filter $(1),$(RECENTLY_ACTIVATED_FEATURES)))
endef

# $(call if-module-guessable,<name> [<name1> ...])
define if-module-guessable
$(if $(1),$(filter $(1),\
	$(if $(findstring all,$(AUTODETECT)), \
		$(notdir $(call filter-dirs,$(wildcard $(DETECTDIR)/*))), \
		$(AUTODETECT))))
endef

# $(call if-feature,<name> [<name1> ...])
# Return a list of features without obsolete and disabled features.
define if-feature
$(if $(1),$(filter-out $(DISABLE_FEATURES) $(ALL_DISABLE_FEATURES),$(1)))
endef

define filter-dirs
$(realpath $(filter %/,$(addsuffix /,$(1))))
endef

# $(call expand-list,<variable-prefix>,<suffix-list>)
# Return a content of $(<variable-prefix>-<suffix>)
define expand-list
$(if $(1),$(foreach n,$(2),$(n) $(call expand-list,$(1),$($(1)-$(n)))))
endef

# $(call expand-features,<variable-prefix>,<feature-list>)
# Return a list of the contents of the $(<variable-prefix>-<feature>) variables.
# The list is expanded recursively.
define expand-features
$(if $(1),\
	$(foreach n,$(call if-feature,$(2)),\
		$(n) $(call if-feature,$(call expand-features,$(1),$($(1)-$(n))))))
endef

# $(call expand-disabled-features,<feature-list>)
# Return a list of features that disable the specified features.
define expand-disabled-features
$(foreach n,$(call if-feature,$(1)),$(FEATURE-DISABLES-$(n)))
endef

# $(call include-features-once,<feature-list>)
define include-features-once
$(if $(1),$(eval USED_FEATURES = ))
$(if $(1),\
	$(foreach n,$(1),\
		$(call include-feature,$(n),$(wildcard \
			$(filter-out $(MAKEFILE_LIST),$(n:%=$(FEATURESDIR)/%/rules.mk))))))
endef

define _word-position
$(if $(filter $(strip $(1)),$(word 1,$(3))),$(words $(3)),$(if $(filter 1,$(words $(2))),,$(call _word-position,$(1), $(wordlist 2,$(words $(2)),$(2)), $(firstword $(2)) $(3))))
endef

# $(call word-position,<word>,<list>)
# Returns the position in the list or empty.
define word-position
$(strip $(call _word-position,$(1),$(2) x))
endef

# $(call set-sysconfig,<filename>,<varname>,<value>)
define set-sysconfig
$(eval n-file := $(call word-position,$(1),$(SYSCONFIG_FILES) $(1)))\
$(eval n-name := $(call word-position,$(2),$(SYSCONFIG_NAMES_$(n-file)) $(2)))\
$(if $(filter $(1),$(SYSCONFIG_FILES)),,$(eval SYSCONFIG_FILES += $(1)))\
$(if $(filter $(2),$(SYSCONFIG_NAMES_$(n-file))),,$(eval SYSCONFIG_NAMES_$(n-file) += $(2)))\
$(eval SYSCONFIG_$(n-file)_$(n-name) = $(subst $$,$$$$,$(3)))\
$(eval undefine n-file)\
$(eval undefine n-name)
endef

# $(shell-export-vars)
# The function generates shell-code which exports all make variables that can be
# used in the shell.
#
# Example: $(shell $(shell-export-vars) /usr/bin/some-helper)
#
define shell-export-vars
$(foreach v,$(shell echo '$(.VARIABLES)' | tr ' ' '\n' | grep '^[A-Z][A-Za-z0-9_]*$$'),export $(v)='$(subst ','\'',$($(v)))';)
endef

endif # __included-functions-mk
