Name: make-initrd
Version: 0.1.4
Release: alt1

Summary: Creates an initramfs image
License: GPL3
Group: System/Base

Packager: Alexey Gladkov <legion@altlinux.ru>

Requires: libshell make sed module-init-tools coreutils grep glibc-utils
Requires: make-initrd-klibc ash-klibc
Requires: udev-initramfs module-init-tools-initramfs

BuildRequires: klibc-devel

# This avoids getting a dependency on sh from "#!/bin/sh".
#AutoReq: yes, nopam, noperl, nopython, noshell, notcl
AutoReq: noshell

Source0: %name-%version.tar

%description
make-initrd is a new, uevent-driven initramfs infrastructure based around udev.

%package klibc
Summary: Unilities for %name
Group: System/Base
Requires(pre): klibc-utils-initramfs

%description klibc
Unilities for %name.

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
%make_install DESTDIR=%buildroot install
mkdir -p -- %buildroot/lib/mkinitrd/initramfs/{bin,sbin,lib}
mv -f -- %buildroot/lib/mkinitrd/klibc/bin/* %buildroot/lib/mkinitrd/initramfs/bin/

%files
%config(noreplace) %_sysconfdir/initrd.mk
%_bindir/*
%_datadir/*
%exclude %_datadir/%name/features/lvm
%doc docs/README.ru

%files klibc
/lib/mkinitrd/initramfs

%files lvm
%_datadir/%name/features/lvm

%changelog
* Thu Sep 03 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.4-alt1
- Fix requires;
- Fix deadlock;
- Fix parsing /proc/cmdline.
- Add docs/README.ru.

* Wed Aug 26 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.3-alt1
- Move klibc utilities from /lib/mkinitrd/klibc/bin/
  to /lib/mkinitrd/initramfs/bin/.
- Accept "3" as runlevel in command line (ALT#21103).
- Fix resume from disk (ALT#21102).

* Mon Aug 03 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.2-alt1
- Increase verbosity.
- WORKDIR is kernel-depended now.
- cleanup: Fix dependencies.

* Mon Aug 03 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.1-alt1
- Ignore modules options on copying (ALT #20936).

* Fri May 29 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.0-alt1
- First build.
