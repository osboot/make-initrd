GUESS_COMMON_SCRIPTS := $(call wildcard-features,guess/common)

GUESSv1_COMMON_SOURCE := $(current-feature-name)
GUESSv1_COMMON_RULE = EMPTYVAR{GUESS_COMMON_SCRIPTS}==false ACTION{RUN}=$(FEATURESDIR)/common/bin/guess
