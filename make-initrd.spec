Name: make-initrd
Version: 0.1.0
Release: alt1

Summary: Creates an initramfs image
License: GPL3
Group: System/Base

Packager: Alexey Gladkov <legion@altlinux.ru>

Requires: libshell make sed module-init-tools coreutils grep glibc-utils

# This avoids getting a dependency on sh from "#!/bin/sh".
#AutoReq: yes, nopam, noperl, nopython, noshell, notcl
AutoReq: noshell

Source0: %name-%version.tar

%description
Creates an initramfs image

%package lvm
Summary: LVM module for %name
Group: System/Base
Requires: lvm2

%description lvm
LVM module for %name

%prep
%setup -q

%build
%make_build

%install
%makeinstall

%files
%_sysconfdir/*
%_bindir/*
%_datadir/*
%exclude %_datadir/%name/features/lvm

%files lvm
%_datadir/%name/features/lvm

%changelog
* Fri May 29 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.0-alt1
- Fist build.
