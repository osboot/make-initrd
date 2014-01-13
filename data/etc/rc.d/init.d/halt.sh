#!/bin/sh
### BEGIN INIT INFO
# Provides:            halt
# Required-Start:      killall
# Should-Start:
# Required-Stop:
# Should-Stop:
# Default-Start:       0 2 6
# Default-Stop:
# Short-Description:   It kills all processes.
# Description:         It kills all processes and unmounts file systems.
# X-LFS-Provided-By:   LFS
### END INIT INFO

if [ -n "$1" -a "$1" != start ]; then
	exit 0
fi

WITHOUT_RC_COMPAT=1

# Source function library.
. /etc/init.d/functions
. /etc/init.d/functions-addon

HALTCMD=
SourceIfNotEmpty /etc/sysconfig/halt
[ -x "$HALTCMD" ] || HALTCMD=/sbin/poweroff

case "${INIT_HALT-}" in
	HALT) HALTCMD=/sbin/halt ;;
	POWEROFF) HALTCMD=/sbin/poweroff ;;
esac

unset action

action_begin_msg()
{
	[ -z "$*" ] || printf '%s ' "$*"
}

action_end_msg()
{
	local rc=$1
	if [ "$BOOTUP" = color ]; then
		[ $rc = 0 ] && echo_success || echo_failure
	fi
	echo
}

action_passed_msg()
{
	[ "$BOOTUP" != color ] ||
		echo_passed
	echo
}

action()
{
	action_begin_msg "$1"
	shift
	$*
	local rc=$?
	action_end_msg "$rc"
	return $rc
}

sendsigs()
{
	action_begin_msg 'Asking all remaining processes to terminate'
	killall5 -15
	local rc=$?
	if [ "$rc" = 2 ]; then
		action_passed_msg
		return
	else
		action_end_msg "$rc"
	fi

	local seq=5
	if [ "$rc" = 0 ]; then
		for seq in 1 2 3 4 5; do
			killall5 -18 || break
			sleep 1
		done
	fi

	[ "$seq" = 5 ] || return 0
	action_begin_msg 'Killing all remaining processes'
	killall5 -9
	rc=$?
	if [ "$rc" = 2 ]; then
		action_passed_msg
	else
		action_end_msg "$rc"
	fi
}

# Unmount file systems, killing processes if we have to.
UnmountFilesystems()
{
	local sig= retry delay findmnt msg= points

	retry="$1"; shift
	delay="$1"; shift
	findmnt="$1"; shift

	points="$($findmnt)"

	while [ -n "$points" -a "$retry" -gt 0 ]; do
		dsort points $points

		[ "$retry" -eq 3 ] || msg='(retry) '

		for point in $points; do
			action "Unmounting filesystem $msg[$point]:" umount -flt noproc "$point"
		done

		points="$($findmnt)"
		[ -n "$points" ] || break

		[ "${BOOTUP-}" != verbose ] || fuser -mv $points
		fuser -km $sig $points >/dev/null

		sleep $delay

		sig=-9
		retry=$(($retry-1))
	done
}

# Kill all processes
sendsigs

# Unmount non-/dev tmpfs.
non_dev_tmpfs()
{
	findmnt -mno TARGET -t tmpfs |
	while read point; do
		[ "$point" = '/dev' ] || printf '%s ' "$point"
	done
}
UnmountFilesystems 3 5 non_dev_tmpfs

# Turn off swap, then unmount file systems.
while read d x; do
	case "$d" in
		Filename) ;;
		*) swapoff "$d" ;;
	esac
done < /proc/swaps

# Unmount all the rest.
all_rest()
{
	findmnt -mno SOURCE,TARGET |
	while read s t; do
		case "$s:$t" in
			none:*|*:/|*:/dev|*:/proc|*:/sys|*:/dev/*|*:/proc/*|*:/sys/*) ;;
			*) printf '%s\n' "$t" ;;
		esac
	done
}
UnmountFilesystems 3 5 all_rest
