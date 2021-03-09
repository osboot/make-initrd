.\" Copyright (C) 2021  Alexey Gladkov <gladkov.alexey@gmail.com>
.\"
.\" Additional documentation for the initrd-scanmod.
.\"
.\" This file is covered by the GNU General Public License,
.\" which should be included with make-initrd as the file LICENSE.

[NAME]
initrd-scanmod \- searches for kernel modules according to patterns.

[DESCRIPTION]
The utility uses a set of
.I rules
to find and show kernel modules. It shows the modules that satisfy
.B all
the
.I rules.
This utility reads
.I rules
for filtering from files. It can filter modules by the following criteria:
.I "alias"
,
.I "author"
,
.I "depends"
,
.I "description"
,
.I "filename"
,
.I "firmware"
,
.I "license"
,
.I "name"
and
.I "symbol".

[RULES]
The rules are in the format:
.PP
.in +4n
.EX
<keyword> <regular-expression>
.EE
.in
.PP
All empty rules or starting with the ``#'' character are ignored. If the keyword
begins with the prefix ``not-'' then the rule will be inverted.
.SS name
This allows you to filter modules by the kernel module name.
.SS alias
This allows you to search the aliases list of a kernel module. For example:
.PP
.in +4n
.EX
alias ^fs-
.EE
.in
.PP
.SS depends
Modules depend on other modules. This keyword allows you to filter modules based
on dependencies. Example:
.PP
.in +4n
.EX
depends ^(.*,)?af_alg(,.*)?$
.EE
.in
.PP
.SS symbol
Also you can filter by module symbol names. This is a very shaky approach. The
symbol names may change in the new kernel version and the filter will stop working.
Some stable subsystems don't change often. Example:
.PP
.in +4n
.EX
symbol ^(nfs_.*|svc_.*)$
.EE
.in
.PP
or
.PP
.in +4n
.EX
symbol ^register_virtio_driver
.EE
.in
.PP

[SEE ALSO]
.BR modinfo (1),
.BR modprobe (1),
.BR depinfo (1)
