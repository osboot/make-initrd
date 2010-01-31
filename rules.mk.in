.EXPORT_ALL_VARIABLES:

# Features include helper: skip a feature which has already loaded
require = $(foreach req,$(1), \
		$(eval include $(filter-out $(MAKEFILE_LIST), \
				$(realpath $(req:%=$(FEATURESDIR)/%/rules.mk)))))

ifneq "$(strip $(INITRD_CONFIG))" ''

# Load user configuration
include $(INITRD_CONFIG)

# Load guessed parameters
ifeq ($(MAKECMDGOALS),genimage)
include $(WORKDIR)/guess/guessed.mk
endif

# Load requested features
ifneq "$(strip $(FEATURES))" ''
include $(call require,$(FEATURES))
endif # FEATURES

endif # INITRD_CONFIG

all:
	@for c in $(INITRD_CONFIG_LIST); do \
		echo "Config file: $$c"; \
		wsuffix="$${c##*/}"; \
		wsuffix="$${wsuffix%.mk}"; \
		export WORKDIR_SUFFIX="$$wsuffix"; \
		export INITRD_CONFIG="$$c"; \
		$(TOOLSDIR)/run-make $(MAKE) $(MFLAGS) -f $(MAKE_INITRD_BIN) guess; \
		$(TOOLSDIR)/run-make $(MAKE) $(MFLAGS) -f $(MAKE_INITRD_BIN) genimage; \
	done

genimage: install
	@echo
	@echo "Image is saved as $(IMAGEFILE)"
	@echo

check-for-root:
	@if [ "$$(id -u)" != 0 ]; then \
	    echo "Only root can do that"; \
	    exit 1; \
	fi

depmod-host: check-for-root
	@echo "Generating module dependencies on host ..."
	$Qdepmod -a -F "/boot/System.map-$(KERNEL)" "$(KERNEL)"

create: depmod-host
	@echo "Creating initrd image ..."
	@mkdir -m 755 -p $(verbose) -- $(WORKDIR) $(ROOTDIR)
	@$(CREATE_INITRD)

rescue-modules: create
	@if [ -n "$(RESCUE_MODULES)" ]; then \
	    echo "Installing resue modules ..."; \
	    $(ADD_RESCUE_MODULES) $(RESCUE_MODULES); \
	fi

pack: create rescue-modules
	@echo "Packing image to archive ..."
	@$(PACK_IMAGE)

install: pack
	@echo "Installing image ..."
	@mv -f $(verbose) -- "$(WORKDIR)/initrd.img" "$(IMAGEFILE)"

clean:
	@echo "Removing work directory ..."
	$Qrm -rf -- "$(WORKDIR)"

guess:
	@mkdir -m 755 -p -- $(WORKDIR)/guess
	@cat /dev/null > $(WORKDIR)/guess/modules
	@cat /dev/null > $(WORKDIR)/guess/modalias
	@cat /dev/null > $(WORKDIR)/guess/features
	@for i in $(AUTODETECT); do \
		if [ -z "$${i##*:*}" ]; then \
			"$(TOOLSDIR)/guess-$${i%%:*}" "$${i#*:}"; \
		else \
			"$(TOOLSDIR)/guess-$$i"; \
		fi;\
	done
	@$(TOOLSDIR)/guess-config > $(WORKDIR)/guess/guessed.mk

guess-config: AUTODETECT ?= common root resume
guess-config: guess check-for-root
	@cat $(WORKDIR)/guess/guessed.mk
	$Qrm -rf -- "$(WORKDIR)"

bug-report: check-for-root
	@mkdir -m 755 -p -- $(WORKDIR)
	@$(TOOLSDIR)/bug-report
	$Qrm -rf -- "$(WORKDIR)"
