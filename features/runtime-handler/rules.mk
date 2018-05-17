runtime-handler:
	@$(MSG) "Adding runtime handler support ..."
	@put-tree "$(ROOTDIR)" "$(FEATURESDIR)/$@/data"

pack: runtime-handler
