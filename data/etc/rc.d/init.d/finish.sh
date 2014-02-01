#!/bin/sh
### BEGIN INIT INFO
# Provides:            finish
# Required-Start:
# Should-Start:
# Required-Stop:
# Should-Stop:
# Default-Start:       0 2 6
# Default-Stop:
# Short-Description:   Stop all remaining subsystems.
# Description:         Stop all remaining subsystems.
# X-LFS-Provided-By:   LFS
### END INIT INFO

[ "$1" = start ] || exit 0

# Bring down all unneeded services that are still running (there shouldn't 
# be any, so this is just a sanity check)
sys=/var/lock/subsys
for i in "$sys"/*; do
	# Check if the script is there.
	[ -f "$i" ] || continue

	# Don't run *.rpm* and *~ scripts
	[ "${i%.rpm*}" = "$i" -a "${i%\~}" = "$i" ] || continue

	# Get the subsystem name.
	subsys="${i#$sys/}"

	# Bring the subsystem down.
	x="/etc/init.d/$subsys"
	if [ -x "$x" ]; then
		"$x" stop
	else
		rm -f "$i"
	fi
done
