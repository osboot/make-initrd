pipeline: create
	@$(MSG) "Adding pipeline support ..."
	@put-tree "$(ROOTDIR)" $(PIPELINE_DATADIR)

pack: pipeline
