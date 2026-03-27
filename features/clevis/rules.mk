PUT_FEATURE_PROGS += \
    clevis \
    clevis-decrypt \
    clevis-decrypt-tpm2 \
    clevis-luks-unlock \
    clevis-pin-tpm2 \
    cryptsetup \
    jq \
    jose \
    tpm2_pcrread \
    tpm2_getcap

PUT_FEATURE_FILES += /usr/bin/clevis-luks-common-functions

PUT_FEATURE_LIBS += libtss2-tcti-device.so.0
