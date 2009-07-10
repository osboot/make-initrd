Name: initramfs-data
Version: 0.0.1
Release: alt1

Summary: Initramfs data files
License: GPL3
Group: System/Base
BuildArch: noarch

Packager: Alexey Gladkov <legion@altlinux.ru>

# This avoids getting a dependency on sh from "#!/bin/sh".
AutoReq: yes, nopam, noperl, nopython, noshell, notcl

Source0: %name-%version.tar

%description
Initramfs data files

%prep
%setup -q

%install
%makeinstall

%files
%_datadir/*
%changelog
* Fri May 29 2009 Alexey Gladkov <legion@altlinux.ru> 0.0.1-alt1
- Fist build.
