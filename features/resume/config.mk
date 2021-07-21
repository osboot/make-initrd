GUESSv1_RESUME_SOURCE := $(current-feature-name)
GUESSv1_RESUME_RULE = EXISTS{$(PROCFS_PATH)/cmdline} ACTION{RUN}=$(FEATURESDIR)/resume/bin/guess
