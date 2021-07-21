LKRG_DATADIR 	?= $(FEATURESDIR)/lkrg/data

GUESSv1_LKRG_SOURCE := $(current-feature-name)
GUESSv1_LKRG_RULE = KMODULE{p_lkrg}==enabled VAR{FEATURES}+=lkrg
