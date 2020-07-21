$(call feature-requires,devmapper)

LVM_BIN		?= lvm
LVM_CONF	?= $(SYSCONFDIR)/lvm/lvm.conf
LVM_DATADIR	?= $(FEATURESDIR)/lvm/data
