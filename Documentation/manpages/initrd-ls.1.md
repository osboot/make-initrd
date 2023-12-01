initrd-ls(1)

# NAME

initrd-ls - shows contents of initrd archive

# SYNOPSIS

*initrd-ls* [options] initramfs

# DESCRIPTION

Displays initramfs contents in a format similar to ls command. If initramfs
contains more than one cpio archive, utility will show all of them. Archives
can be compressed. In this case, utility will take a look inside.

# OPTIONS

*--no-mtime*
	Hide modification time.

*-b, --brief*
	Show only brief information about archive parts.

*-n, --name*
	Show only filenames.

*-C, --compression*
	Show compression method for each archive.

*-V, --version*
	Show version of program and exit.

*-h, --help*
	Show this text and exit.

# AUTHOR

Written by Alexey Gladkov.

# BUGS

Report bugs to the authors.
