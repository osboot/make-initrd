Name: make-initrd
Version: 0.1.6
Release: alt1

Summary: Creates an initramfs image
License: GPL3
Group: System/Base

Packager: Alexey Gladkov <legion@altlinux.ru>

Requires: libshell make sed module-init-tools coreutils grep glibc-utils
Requires: kinit-utils

# This avoids getting a dependency on sh from "#!/bin/sh".
#AutoReq: yes, nopam, noperl, nopython, noshell, notcl
AutoReq: noshell, noshebang

Source0: %name-%version.tar

%description
make-initrd is a new, uevent-driven initramfs infrastructure based around udev.

%package lvm
Summary: LVM module for %name
Group: System/Base
Requires: lvm2
AutoReq: noshell, noshebang

%description lvm
LVM module for %name

%package luks
Summary: LUKS module for %name
Group: System/Base
Requires: cryptsetup
AutoReq: noshell, noshebang

%description luks
LUKS module for %name

%prep
%setup -q

%build
%make_build

%install
%make_install DESTDIR=%buildroot install

%files
%config(noreplace) %_sysconfdir/initrd.mk
%_bindir/*
%_datadir/*
%exclude %_datadir/%name/features/lvm
%exclude %_datadir/%name/features/luks
%doc docs/README.ru

%files lvm
%_datadir/%name/features/lvm

%files luks
%_datadir/%name/features/luks

%changelog
* Wed Nov 11 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.6-alt1
- Remove klibc support.

* Sun Oct 25 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.5-alt1
- Add simple RAID feature.
- Add simple NFS root support.
- Add simple network configuration.
- Add LUKS implemetation.
- Use udevsh in all udev helpers.
- device-mapper: Load dm_mod before udev.
- add-modules: Add preload-modules and load-modules stage.
- Allow to set more than one parameter with the same name.
- mkinitrd-like make-initrd wrapper (thx Alexey I. Froloff).
- RPM: Do not generate automatic requires from shebang.

* Fri Sep 04 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.4-alt2
- Fix ugly bug in cmdline parser.

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
