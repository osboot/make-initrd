#!/bin/sh
### BEGIN INIT INFO
# Provides:            killall
# Required-Start:
# Should-Start:
# Required-Stop:       localnet
# Should-Stop:
# Default-Start:       0 2 6
# Default-Stop:
# Short-Description:   Attempts to kill remaining processes.
# Description:         Attempts to kill remaining processes.
# X-LFS-Provided-By:   LFS
### END INIT INFO

if [ "$1" != start ]; then
	exit 0
fi

# Bring down all unneeded services that are still running (there shouldn't 
# be any, so this is just a sanity check)
for i in /var/lock/subsys/*; do
	# Check if the script is there.
	[ -f "$i" ] || continue

	# Don't run *.rpm* and *~ scripts
	[ "${i%.rpm*}" = "$i" -a "${i%\~}" = "$i" ] || continue

	# Get the subsystem name.
	subsys="${i#/var/lock/subsys/}"

	# Bring the subsystem down.
	if [ -x "/etc/init.d/$subsys" ]; then
		"/etc/init.d/$subsys" stop
	else
		rm -f "$i"
	fi
done
