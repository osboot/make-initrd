.EXPORT_ALL_VARIABLES:

all: initrd

check-for-root:
	@if [ "$$(id -u)" != 0 ]; then \
	    echo "Only root can do that"; \
	    exit 1; \
	fi

prepare:
	@echo "Preparing work directory ..."
	@mkdir -m 700 -p -- $(WORKDIR) $(ROOTDIR)

depmod: check-for-root prepare
	@echo "Generating module dependencies ..."
	depmod -a -F "/boot/System.map-$(KERNEL)" "$(KERNEL)"

create: depmod
	@echo "Creating initrd image ..."
	$(CREATE_INITRD)

run-scripts:
	@echo "Running scripts ..."
	@$(RUN_SCRIPTS)

optional-bootsplash:
	@echo "Building bootsplash ..."
	$(OPTIONAL_BOOTSPLASH)

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
