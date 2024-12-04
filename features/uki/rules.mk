# SPDX-License-Identifier: GPL-3.0-or-later

pack: create
	@$(VMSG) "Sorting sysvinit services ..."
	@$(TOOLSDIR)/sort-services --rcdir="$(ROOTDIR)/etc/rc.d" "$(ROOTDIR)/etc/rc.d/init.d"
	@$(VMSG) "Packing UKI image ..."

install: pack
	@$(MSG) 'Used features: $(USED_FEATURES)'
	@$(MSG_N) 'Packed modules: '
	@find $(ROOTDIR)/lib/modules/$(KERNEL) -type f \( -name '*.ko'  -o -name '*.ko.*' \) -printf '%f\n' 2>/dev/null | \
	    sed -e 's/\.ko\(\.[^\.]\+\)\?$$//' | sort -u | tr '\n' ' '
	@printf '\n'
