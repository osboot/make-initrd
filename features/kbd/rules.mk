kbd:
	@$(MSG) "Adding kbd support ..."
	@for s in gzip:gz bzip2:bz2 xz:xz; do \
		[ -z "$$(find "$(KBD_FONTS_DIR)" "$(KBD_KEYMAPS_DIR)" -type f -name "*.$${s#*:}" -print -quit)" ] || \
			put-file "$(ROOTDIR)" `which $${s%:*}`; \
	done
	@put-file "$(ROOTDIR)" $(KBD_FILES)
	@put-tree "$(ROOTDIR)" $(KBD_DATADIR)
	@mkdir -p -- "$(ROOTDIR)/etc/sysconfig"
	@cat $(KBD_CONFIGS) > "$(ROOTDIR)/etc/sysconfig/console"

pack: kbd
