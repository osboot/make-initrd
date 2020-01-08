#!/bin/sh -efu

export PATH="$HOME/src/utils:$PATH"

read -r testcase < "$HOME/src/.tests/cur"

[ ! -x "$HOME/src/tests/$testcase/run.sh" ] ||
	exec "$HOME/src/tests/$testcase/run.sh"

rc=0
for n in build-prepare build-initrd build-finish; do
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
