initrd-put(1)

# NAME

initrd-put - copies files and directories into a specified directory

# SYNOPSIS

*initrd-put* [<options>] <destdir> directory [directory ...]++
*initrd-put* [<options>] <destdir> file [file ...]

# DESCRIPTION

Utility allows to copy files and directories along with their dependencies into
a specified destination directory. This utility follows symbolic links and
binary dependencies and copies them along with the specified files.

# OPTIONS

*-n, --dry-run*
	don’t do nothing.

*-e, --exclude=*_REGEXP_
	exclude files matching _REGEXP_.

*-f, --force*
	overwrite destination file if exists.

*-l, --log=*_FILE_
	white log about what was copied.

*-r, --remove-prefix=*_PATH_
	ignore prefix in path.

*-v, --verbose*
	print a message for each action.

*-V, --version*
	Show version of program and exit.

*-h, --help*
	Show this text and exit.

# AUTHOR

Written by Alexey Gladkov.

# BUGS

Report bugs to the authors.
