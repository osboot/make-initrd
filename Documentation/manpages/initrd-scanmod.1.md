initrd-scanmod(1)

# NAME

initrd-scanmod - searches for kernel modules according to patterns

# SYNOPSIS

*initrd-scanmod* [options] [--] rules-file [rules-file ...]

# DESCRIPTION

The utility uses a set of rules to find and show kernel modules. It shows the
modules that satisfy all the rules.  This utility reads rules for filtering from
files. It can filter modules by the following criteria: *alias*, *author*,
*depends*, *description*, *filename*, *firmware*, *license*, *name* and *symbol*.

# OPTIONS

*-k, --set-version=*_VERSION_
	use _VERSION_ instead of ‘uname -r‘.

*-b, --base-dir=*_DIR_
	use _DIR_ as filesystem root for /lib/modules;

*-v, --verbose*
	print a message for each action.

*-V, --version*
	Show version of program and exit.

*-h, --help*
	Show this text and exit.

# RULES

The rules are in the format:

```
<keyword> <regular-expression>
```

All empty rules or starting with the ‘‘#’’ character are ignored. If the keyword
begins with the prefix ‘‘not-’’ then the rule will be inverted.

	*name*

	This allows you to filter modules by the kernel module name.

	*alias*

	This allows you to search the aliases list of a kernel module. For
	example:

	```
	alias ˆfs-
	```

	*depends*

	Modules depend on other modules. This keyword allows you to filter
	modules based on dependencies. Example:

	```
	depends ˆ(.*,)?af_alg(,.*)?$
	```

	*symbol*

	Also you can filter by module symbol names. This is a very shaky
	approach. The symbol names may change in the new kernel version and the
	filter will stop working. Some stable subsystems don’t change often.
	Example:

	```
	symbol ˆ(nfs_.*|svc_.*)$
	```

	or

	```
	symbol ˆregister_virtio_driver
	```

# AUTHOR

Written by Alexey Gladkov.

# BUGS

Report bugs to the authors.
