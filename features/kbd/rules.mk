kbd:
	@$(MSG) "Adding kbd support ..."
	@mkdir -p -- "$(ROOTDIR)/etc/sysconfig"
	@$(FEATURESDIR)/kbd/bin/get-config > "$(ROOTDIR)/etc/sysconfig/console"
	@put-file "$(ROOTDIR)" $(KBD_FILES)
	@put-tree "$(ROOTDIR)" $(KBD_DATADIR)
	@. "$(ROOTDIR)/etc/sysconfig/console"; \
	[ -z "$$FRAMEBUFFER_MODULE" ] || \
		$(TOOLSDIR)/add-module "$$FRAMEBUFFER_MODULE"; \
	[ -z "$${CONSOLE_FONT##/*}" ] || \
		find "$(KBD_FONTS_DIR)"   -name "$$CONSOLE_FONT.*"        -exec put-file "$(ROOTDIR)" '{}' '+'; \
	[ -z "$${CONSOLE_FONT_UNIMAP##/*}" ] || \
		find "$(KBD_UNIMAPS_DIR)" -name "$$CONSOLE_FONT_UNIMAP.*" -exec put-file "$(ROOTDIR)" '{}' '+'; \
	[ -z "$${CONSOLE_KEYMAP##/*}" ] || \
		find "$(KBD_KEYMAPS_DIR)" -name "$$CONSOLE_KEYMAP.*"      -exec put-file "$(ROOTDIR)" '{}' '+'; \
	find "$(KBD_KEYMAPS_DIR)" -name 'include' -exec put-file "$(ROOTDIR)" '{}' '+';
	@for s in gzip:gz bzip2:bz2 xz:xz; do \
		[ -z "$$(find "$(ROOTDIR)$(KBD_DATA_DIR)" -type f -name "*.$${s#*:}" -print -quit)" ] || \
			put-file "$(ROOTDIR)" `which $${s%:*}`; \
	done

pack: kbd
