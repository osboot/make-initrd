$(call feature-requires,usb)

SMART_CARD_BIN		?= pcscd pkcs11-tool opensc-tool pkcs15-tool pkcs15-init pkcs15-crypt pcsc_scan
SMART_CARD_DATADIR	?= $(FEATURESDIR)/smart-card/data
SMART_CARD_FILES	?= $(SYSCONFDIR)/opensc\
			   $(SYSCONFDIR)/reader.conf.d\
			   $(LIBDIR)/pcsc
