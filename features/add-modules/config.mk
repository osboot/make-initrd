$(call feature-requires,depmod-image add-udev-rules)

KMODDEPSDIR := $(FEATURESDIR)/add-modules/kmodule.deps.d

MODULES_ADD     ?=
MODULES_TRY_ADD ?=
MODULES_LOAD    ?=
MODULES_PRELOAD ?=
RESCUE_MODULES  ?=

MODULES_PATTERN_SETS ?=
