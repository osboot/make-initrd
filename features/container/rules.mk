container:
	@echo "Adding container support ..."
	@put-file "$(ROOTDIR)" $(CONTAINER_FILES)
	@put-tree "$(ROOTDIR)" $(CONTAINER_DATADIR)

pack: container
