MODULES_TRY_ADD += $(PIPELINE_MODULES)

pipeline: create
	@$(MSG) "Adding pipeline support ..."
	@put-file "$(ROOTDIR)" $(PIPELINE_FILES)
	@put-tree "$(ROOTDIR)" $(PIPELINE_DATADIR)

pack: pipeline
