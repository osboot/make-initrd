#
# Search for options:
#   comment="x-initrd-mount"
#   x-initrd-mount
#
fstab: create
	@echo "Adding fstab ..."
	$Q mkdir -p -- "$(ROOTDIR)"/etc
	$Q :>"$(ROOTDIR)"/etc/fstab
	$Q grep '^[[:space:]]*[^#].\+[[:space:]]/[[:space:]]' "$(FSTAB)" >> "$(ROOTDIR)"/etc/fstab ||:
	$Q grep '^[[:space:]]*[^#].\+\<x-initrd-mount\>'      "$(FSTAB)" >> "$(ROOTDIR)"/etc/fstab ||:
	$Q sort -uo "$(ROOTDIR)"/etc/fstab "$(ROOTDIR)"/etc/fstab

pack: fstab
