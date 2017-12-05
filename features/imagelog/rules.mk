IMAGEFILES  = $(WORKDIR)/image.files

imagelog: pack
	@echo "Writing image log files ..."
	$Qmkdir -p -- "$(IMAGELOGDIR)/$(KERNEL).$(WORKDIR_SUFFIX)"
	$Qprintf '%s\n' $(FEATURES) | \
	    sort -uo "$(IMAGELOGDIR)/$(KERNEL).$(WORKDIR_SUFFIX)/features"
	$Qmv -- "$(IMAGEFILES)" "$(IMAGELOGDIR)/$(KERNEL).$(WORKDIR_SUFFIX)/files"

install: imagelog
