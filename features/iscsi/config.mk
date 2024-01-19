$(call feature-requires,network)

ISCSI_MODULES = iscsi-ibft \
                     iscsi-tcp \
                     iscsi-boot-sysfs \
                     iscsi-target-mod \
                     target-core-mod \
                     libiscsi \
                     libiscsi-tcp \
                     scsi-transport-iscsi \
                     sd_mod \
                     sr_mod

ISCSI_DATADIR = $(FEATURESDIR)/iscsi/data
