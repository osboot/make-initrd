IMAGEFILES  = $(WORKDIR)/image.files

imagelog: pack
	@echo "Writing image log files ..."
	$Qmkdir -p -- "$(IMAGELOGDIR)/$(KERNEL).$(WORKDIR_SUFFIX)"
	$Qprintf '%s\n' $(FEATURES) | \
	    sort -uo "$(IMAGELOGDIR)/$(KERNEL).$(WORKDIR_SUFFIX)/features"
	$Qsort -uo "$(IMAGELOGDIR)/$(KERNEL).$(WORKDIR_SUFFIX)/files" "$(IMAGEFILES)"
	$Qrm -f -- "$(IMAGEFILES)"

install: imagelog
