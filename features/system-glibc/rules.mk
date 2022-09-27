ifeq ($(INITRD_NO_LIBNSS),)
SYSTEM_GLIBC_LIBNSS_BIN   := $(FEATURESDIR)/system-glibc/bin/system-glibc-libnss
SYSTEM_GLIBC_LIBNSS_FILES := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_LIBNSS_BIN) files)
SYSTEM_GLIBC_LIBNSS_DIRS  := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_LIBNSS_BIN) dirs)

PUT_FEATURE_FILES += $(SYSTEM_GLIBC_LIBNSS_FILES)
PUT_FEATURE_DIRS  += $(SYSTEM_GLIBC_LIBNSS_DIRS)
endif

ifeq ($(INITRD_NO_LIBGCC_S),)
SYSTEM_GLIBC_LIBGCC_S_BIN   := $(FEATURESDIR)/system-glibc/bin/system-glibc-libgcc_s
SYSTEM_GLIBC_LIBGCC_S_FILES := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_LIBGCC_S_BIN) files)
SYSTEM_GLIBC_LIBGCC_S_DIRS  := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_LIBGCC_S_BIN) dirs)

PUT_FEATURE_FILES += $(SYSTEM_GLIBC_LIBGCC_S_FILES)
PUT_FEATURE_DIRS  += $(SYSTEM_GLIBC_LIBGCC_S_DIRS)
endif

ifeq ($(INITRD_NO_PWGR),)
SYSTEM_GLIBC_PWGR_BIN   := $(FEATURESDIR)/system-glibc/bin/system-glibc-pwgr
SYSTEM_GLIBC_PWGR_FILES := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_PWGR_BIN) files)
SYSTEM_GLIBC_PWGR_DIRS  := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_PWGR_BIN) dirs)

PUT_FEATURE_FILES += $(SYSTEM_GLIBC_PWGR_FILES)
PUT_FEATURE_DIRS  += $(SYSTEM_GLIBC_PWGR_DIRS)
endif

ifeq ($(INITRD_NO_NETWORK),)
SYSTEM_GLIBC_NETWORK_BIN   := $(FEATURESDIR)/system-glibc/bin/system-glibc-network
SYSTEM_GLIBC_NETWORK_FILES := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_NETWORK_BIN) files)
SYSTEM_GLIBC_NETWORK_DIRS  := $(shell $(shell-export-vars) $(SYSTEM_GLIBC_NETWORK_BIN) dirs)

PUT_FEATURE_FILES += $(SYSTEM_GLIBC_NETWORK_FILES)
PUT_FEATURE_DIRS  += $(SYSTEM_GLIBC_NETWORK_DIRS)
endif
