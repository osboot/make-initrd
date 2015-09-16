LXC_DATADIR = $(FEATURESDIR)/lxc/data
LXC_FILES = \
	/sbin/shutdown \
	/usr/sbin/init.lxc \
	/etc/lxc/default.conf \
	/usr/lib/lxc/lxc-init \
	/usr/lib/lxc \
	/usr/lib64/lxc/rootfs/README \
	/usr/share/lxc/config \
	/usr/share/lxc/hooks \
	/usr/share/lxc/lxc.functions \
	$(wildcard /usr/bin/lxc-*)
