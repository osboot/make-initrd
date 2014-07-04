SYSVINIT_FILES = \
	/bin/bash \
	/bin/kill \
	/bin/dmesg \
	/bin/touch \
	/bin/sleep \
	/bin/findmnt \
	/usr/bin/flock \
	/usr/bin/ip \
	/usr/bin/which \
	/sbin/killall5 \
	/sbin/initlog \
	/sbin/start-stop-daemon \
	/etc/initlog.conf \
	/var/resolv \
	/usr/share/terminfo/l

# Add libshell
SYSVINIT_FILES += $(wildcard /bin/shell-*)
