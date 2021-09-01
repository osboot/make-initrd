put_pkcs11_name:
	@$(MSG) "Put file with PKCS#11 name ..."
	$Qmkdir -p "$(WORKDIR)/img/.initrd/pkcs11"
	$Qecho $(SMART_CARD_PKCS11_MODULE) > "$(ROOTDIR)/.initrd/pkcs11/pkcs11.module"

pack: put_pkcs11_name

PUT_FEATURE_FILES += $(SMART_CARD_FILES)
PUT_FEATURE_PROGS += $(SMART_CARD_BIN)
PUT_FEATURE_DIRS  += $(SMART_CARD_DATADIR)

ifeq "$(SMART_CARD_PKCS11_MODULE)" ''
SMART_CARD_PKCS11_MODULE = opensc-pkcs11.so
endif

PUT_FEATURE_LIBS += $(SMART_CARD_PKCS11_MODULE)
PUT_UDEV_RULES += $(SMART_CARD_UDEV_RULES)
