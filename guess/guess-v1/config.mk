feature-configs = $(call wildcard-features,config.mk)
-include $(feature-configs)

$(info XXX $(get-all-active-features))
