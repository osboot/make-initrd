#
# Search for options:
#   comment="x-initrd-mount"
#   x-initrd-mount
#
fstab: create
	@echo "Adding fstab ..."
	@mkdir -p -- "$(ROOTDIR)"/etc
	@:>"$(ROOTDIR)"/etc/fstab
	@grep '^[[:space:]]*[^#].\+[[:space:]]/[[:space:]]' "$(FSTAB)" >> "$(ROOTDIR)"/etc/fstab ||:
	@grep '^[[:space:]]*[^#].\+\<x-initrd-mount\>'      "$(FSTAB)" >> "$(ROOTDIR)"/etc/fstab ||:
	@sort -uo "$(ROOTDIR)"/etc/fstab "$(ROOTDIR)"/etc/fstab

pack: fstab
