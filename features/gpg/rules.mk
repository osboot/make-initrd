# SPDX-License-Identifier: GPL-3.0-or-later

ifeq ($(GPG_PROG),)
  $(error gpg utility must be specified in the "GPG_PROG" variable.)
endif

ifeq ($(GPG_PUBKEYS),)
  $(error one or more public gpg keys must be specified in the "GPG_PUBKEYS" variable.)
endif

PUT_FEATURE_PROGS += $(GPG_PROG)

PHONY += gpg

gpg: create
	@$(VMSG) "Putting gpg keyring ..."
	@mkdir -m700 -p -- "$(ROOTDIR)/etc/initrd/gnupg"
	@$(GPG_PROG) --quiet --homedir "$(ROOTDIR)/etc/initrd/gnupg" --import $(GPG_PUBKEYS)
	@[ -e "$(ROOTDIR)"/bin/gpg ] || \
	  ln -s -- "`type -P $(GPG_PROG)`" "$(ROOTDIR)"/bin/gpg
	@rm -f -- "$(ROOTDIR)/etc/initrd/gnupg"/*~

pack: gpg
