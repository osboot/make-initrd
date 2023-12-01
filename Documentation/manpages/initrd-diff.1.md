initrd-diff(1)

# NAME

initrd-diff - compares the listings of two initrd images

# SYNOPSIS

*initrd-diff* [options] [diff-options] <from_image> <to_image>

# DESCRIPTION

Perform a diff on the files contained within different initrd images and show
the result.

Most of the options allowable by GNU _diff_(1) are acceptable. By default *-NrU0* is
used.

# OPTIONS

*-q, --quiet*
	try to be more quiet.

*-v, --verbose*
	print a message for each action.

*-V, --version*
	print program version and exit.

*-h, --help*
	show this text and exit.

# AUTHOR

Written by Evgenii Terechkov.

# BUGS

Report bugs to the authors.
