initrd-scanmod(1)

# NAME

initrd-scanmod - searches for kernel modules according to patterns

# SYNOPSIS

*initrd-scanmod* [options] [--] rules-file [rules-file ...]

# DESCRIPTION

The *initrd-scanmod* utility searches for kernel modules matching one or more sets
of rules and prints their absolute file paths to standard output, one per line.

Each _rules-file_ defines an independent ruleset. Within a single ruleset, all
rules must match (*logical AND*). When multiple rules files are specified, a
module will be printed if it satisfies the criteria of any of the rulesets
(*logical OR*). Each matching module is printed at most once.

Empty rules files and files containing only comments are ignored.

# OPTIONS

*-k, --set-version=*_VERSION_
	Use _VERSION_ instead of the version returned by `uname -r`.

*-b, --base-dir=*_DIR_
	Use _DIR_ as the filesystem root for /lib/modules.

*-j, --jobs=*_N_
	Run _N_ worker threads concurrently (default: number of online CPUs).

*-v, --verbose*
	Print verbose informational messages during operation.

*-V, --version*
	Show program version and exit.

*-h, --help*
	Display usage help and exit.

# RULES

A rules file consists of one rule per line. The format of each rule is:

```
[not-]<keyword> <regular-expression>
```

Empty lines and lines beginning with the `#` character are ignored.

Regular expressions are compiled as POSIX Extended Regular Expressions (ERE).
Unless anchored with `^` and `$`, patterns match any substring within the target
field.

If the keyword begins with the `not-` prefix, the match result is inverted: the
rule matches only if the pattern does not match the target field.

The supported keywords are:

*name*
	Matches against the kernel module name (e.g. _ext4_, _virtio_net_).

*filename*
	Matches against the absolute filesystem path of the module file
	(e.g. _/lib/modules/.../kernel/drivers/net/..._).

*alias*
	Matches against the list of module aliases declared by the module.
	For example:

	```
	alias ^fs-
	```

*author*
	Matches against the module author string from the module metadata.

*description*
	Matches against the module description string from the module metadata.

*license*
	Matches against the module license string (e.g. _GPL_).

*depends*
	Matches against the comma-separated list of modules this module depends
	on. For example, to match an exact module name in the dependency list:

	```
	depends ^(.*,)?af_alg(,.*)?$
	```

*firmware*
	Matches against the list of firmware files requested by the module.

*symbol*
	Matches against symbol names. Both exported (provided) and imported
	(required dependency) symbols are checked. For example:

	```
	symbol ^(nfs_.*|svc_.*)$
	```

	or

	```
	symbol ^register_virtio_driver$
	```

# EXIT STATUS

*0*
	Success (even if no matching modules were found).

*>0*
	An error occurred (e.g. invalid arguments, unreadable files, or memory allocation failure).

# EXAMPLES

Find all filesystem modules excluding those under architecture-specific or network paths:

```
$ cat >/tmp/fs.rules <<EOF
alias ^fs-
not-filename .*/kernel/arch/.*
not-filename .*/net/.*
EOF

$ initrd-scanmod -k 6.6.0-alt1 /tmp/fs.rules
```

Find modules that either match the filesystem rules or are virtio device drivers:

```
$ cat >/tmp/virtio.rules <<EOF
symbol ^register_virtio_driver$
EOF

$ initrd-scanmod -k 6.6.0-alt1 /tmp/fs.rules /tmp/virtio.rules
```

# AUTHOR

Written by Alexey Gladkov.

# BUGS

Report bugs to the authors.

# SEE ALSO

*depinfo*(1), *make-initrd*(1)
