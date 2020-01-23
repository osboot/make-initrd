#!/bin/bash -efu

PROG="${0##*/}"
TEMPDIR="${TESTDIR:-/tmp}"

message()
{
	printf '%s\n' "$PROG: $*" >&2
}

fatal()
{
	message "$@"
	exit 1
}

show_usage()
{
	[ -z "$*" ] || message "$*"
	echo "Try \`$PROG --help' for more information." >&2
	exit 1
}

TEMP=`getopt -n "$PROG" -o '' -l 'expect:,process:,logfile:,stdout:,retcode:' -- "$@"` ||
	show_usage
eval set -- "$TEMP"

logfile=
stdout=
retcode=0

declare -a expect_see
declare -a expect_send

expect_n=0

while :; do
	case "$1" in
		--expect) shift
			expect_see[$expect_n]="$1"
			;;
		--process) shift
			expect_process[$expect_n]="$1"
			expect_n=$(($expect_n + 1))
			;;
		--logfile) shift
			logfile="$1"
			;;
		--stdout) shift
			stdout="${1:+1}"
			;;
		--retcode) shift
			retcode="$1"
			;;
		--) shift
			break
			;;
		*)
			fatal "Unknown option: $1"
			;;
	esac
	shift
done

for n in '.in' '.out'; do
	[ -e "$TESTDIR/qemu.fifo$n" ] || mkfifo "$TESTDIR/qemu.fifo$n"
done

QEMU_STDIN="$TESTDIR/qemu.fifo.in"
QEMU_STDOUT="$TESTDIR/qemu.fifo.out"

[ -z "$logfile" ] ||
	:> "$logfile"

eof=
nline=$'\n'
buffer=

"$@" |
while [ -z "$eof" ]; do
	IFS='' read -r -n1 c ||
		eof=1

	if [ -z "$c" ]; then
		c="$nline"
		buffer=
	else
		buffer+="$c"
	fi

	[ -z "$logfile" ] || printf '%s' "$c" >> "$logfile" ||:
	[ -z "$stdout"  ] || printf '%s' "$c" ||:

	for i in ${!expect_see[@]}; do
		[ "$buffer" != "${expect_see[$i]}" ] ||
			eval "${expect_process[$i]}" ||:
	done

	[ -z "$eof" ] ||
		exit $retcode
done < "$QEMU_STDOUT"
