.PHONY: buildinfo

IMAGEFILES  = $(WORKDIR)/image.files

buildinfo: pack
	@$(MSG) "Writing build info files ..."
	$Qmkdir -p -- "$(BUILDINFODIR)/$(KERNEL).$(WORKDIR_SUFFIX)"
	$Qprintf '%s\n' $(FEATURES) | \
	    sort -uo "$(BUILDINFODIR)/$(KERNEL).$(WORKDIR_SUFFIX)/features"
	$Qsort -uo "$(BUILDINFODIR)/$(KERNEL).$(WORKDIR_SUFFIX)/files" "$(IMAGEFILES)"
	$Qrm -f -- "$(IMAGEFILES)"

install: buildinfo
