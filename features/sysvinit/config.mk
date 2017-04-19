SYSVINIT_FILES = \
	/bin/kill \
	/bin/dmesg \
	/bin/runas \
	/bin/touch \
	/bin/sleep \
	/bin/findmnt \
	/usr/bin/ip \
	/usr/bin/which \
	/sbin/killall5 \
	/sbin/start-stop-daemon \
	/var/resolv \
	/usr/share/terminfo/l

# Add libshell
SYSVINIT_FILES += $(wildcard /bin/shell-*)
