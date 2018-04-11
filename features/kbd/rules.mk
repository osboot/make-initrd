kbd:
	@$(MSG) "Adding kbd support ..."
	@put-file "$(ROOTDIR)" $(KBD_FILES)
	@put-tree "$(ROOTDIR)" $(KBD_DATADIR)
	@mkdir -p -- "$(ROOTDIR)/etc/sysconfig"
	@cat $(KBD_CONFIGS) > "$(ROOTDIR)/etc/sysconfig/console"

pack: kbd
