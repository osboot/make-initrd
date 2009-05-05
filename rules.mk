.EXPORT_ALL_VARIABLES:

all: initrd

prepare:
	@echo "Preparing work directory ..."
	@mkdir -m 700 -p -- $(WORKDIR) $(STATEDIR) $(ROOTDIR)

depmod: prepare
	@echo "Generating module dependencies ..."
	@if ! $(CHECK_STATE) $@ check; then \
	    depmod -a -F "/boot/System.map-$(KERNEL)" "$(KERNEL)"; \
	    $(CHECK_STATE) $@ state; \
	fi

create: depmod
	@echo "Creating initrd image ..."
	@if ! $(CHECK_STATE) $@ check; then \
	    $(CREATE_INITRD); \
	    $(CHECK_STATE) $@ state; \
	fi

run-scripts:
	@echo "Running scripts ..."
	@$(RUN_SCRIPTS)

optional-bootsplash:
	@echo "Building bootsplash ..."
	@if [ ! -f "$(OUTFILE)" ] && ! $(CHECK_STATE) $@ check; then \
	    $(OPTIONAL_BOOTSPLASH); \
	    $(CHECK_STATE) $@ state; \
	fi

pack: prepare
	@echo "Packing image to archive ..."
	@$(PACK_IMAGE)

compress: pack
	@echo "Compressing image ..."
	@$(COMPRESS_IMAGE)

install: pack
	@echo "Installing image ..."
	@mv -f -- "$(WORKDIR)/initrd.img" "$(IMAGEFILE)"

clean:
	@echo "Removing work directory ..."
	@rm -rf -- "$(WORKDIR)"

# Load extra rules
-include $(RULESDIR)/*.mk
