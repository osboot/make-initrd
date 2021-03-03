#!/bin/sh -efu

export PATH="$HOME/src/utils:$PATH"

read -r testcase < "$HOME/src/.tests/cur"
read -r mode < "$HOME/src/.tests/mode"

[ ! -x "$HOME/src/tests/$testcase/run.sh" ] ||
	exec "$HOME/src/tests/$testcase/run.sh"

if ! mountpoint -q /run; then
	mkdir -p /run
	mount -t tmpfs tmpfs /run
fi

set -- build-prepare build-initrd build-finish

[ "$mode" != "shell" ] ||
	set -- build-prepare shell build-finish

rc=0
for n in "$@"; do
	x="$HOME/src/tests/$n.sh"

	[ -x "$x" ] ||
		x="$HOME/src/tests/$testcase/$n.sh"

	[ -x "$x" ] ||
		continue

	if ! "$x"; then
		rc=1
		break
	fi
done

[ "$rc" = 0 ] ||
	/bin/bash

exit $rc
