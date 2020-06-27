MODULES_LOAD += $(LUKS_MODULES)

PUT_FEATURE_DIRS  += $(LUKS_DATADIR)
PUT_FEATURE_FILES += $(CRYPTSETUP_BIN)

$(call require,depmod-image)
$(call require,devmapper)
$(call require,modules-crypto-user-api)
$(call require,system-glibc)
