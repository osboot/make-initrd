Name: make-initrd
Version: 0.2.0
Release: alt1

Summary: Creates an initramfs image
License: GPL3
Group: System/Base

Packager: Alexey Gladkov <legion@altlinux.ru>

Requires: libshell make sed module-init-tools coreutils grep glibc-utils
Requires: kinit-utils ash

# guess-kbd
Requires: udev >= 149-alt1

# This avoids getting a dependency on sh from "#!/bin/sh".
#AutoReq: yes, nopam, noperl, nopython, noshell, notcl
AutoReq: noshell, noshebang

Source0: %name-%version.tar

%description
make-initrd is a new, uevent-driven initramfs infrastructure based around udev.

%package devmapper
Summary: device-mapper module for %name
Group: System/Base
Requires: dmsetup >= 1.02.39-alt2
AutoReq: noshell, noshebang

%description devmapper
device-mapper module for %name

%package lvm
Summary: LVM module for %name
Group: System/Base
Requires: %name-devmapper = %version-%release
Requires: lvm2
AutoReq: noshell, noshebang

%description lvm
LVM module for %name

%package luks
Summary: LUKS module for %name
Group: System/Base
Requires: %name-devmapper = %version-%release
Requires: cryptsetup
AutoReq: noshell, noshebang

%description luks
LUKS module for %name

%package nfs
Summary: NFS module for %name
Group: System/Base
AutoReq: noshell, noshebang

%description nfs
NFS module for %name

%prep
%setup -q

%build
%make_build

%install
%make_install DESTDIR=%buildroot install

mkdir -p %buildroot/%_sysconfdir/sysconfig
echo "MKINITRD=%_sbindir/mkinitrd-make-initrd" > %buildroot/%_sysconfdir/sysconfig/installkernel

%files
%dir %_sysconfdir/initrd.mk.d
%config(noreplace) %_sysconfdir/initrd.mk
%config(noreplace) %_sysconfdir/sysconfig/installkernel
%_bindir/*
%_sbindir/*
%_datadir/*
%exclude %_datadir/%name/features/devmapper
%exclude %_datadir/%name/features/lvm
%exclude %_datadir/%name/features/luks
%exclude %_datadir/%name/features/nfsroot
%doc docs/README.ru

%files devmapper
%_datadir/%name/features/devmapper

%files lvm
%_datadir/%name/features/lvm

%files luks
%_datadir/%name/features/luks

%files nfs
%_datadir/%name/features/nfsroot

%changelog
* Wed Feb 03 2010 Alexey Gladkov <legion@altlinux.ru> 0.2.0-alt1
- make-initrd: Add new arguments:
  + guess-config: guessing configuration;
  + bug-report: helps to generate an error report to developers.
- Introduce new flexible system for guessing configuration.
- Add BLACKLIST_MODULES variable.
- Add installkernel support.
- Fix firmware dirs (thx Valery Inozemtsev).
- Check /lib/udev/vol_id availability.
- Move make-initrd and mkinitrd-make-initrd to sbindir.

* Thu Dec 17 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.7-alt2
- Fix handling of ROOTFLAGS variable.

* Thu Dec 03 2009 Kirill A. Shutemov <kas@altlinux.org> 0.1.7-alt1
- Allow to pack few images by single make-initrd execution.
- Change boottime output and rename modules.
- Add support for /dev/disk/by-{uuid,label}/* devices.
- Fix device-mapper support.
- Rename feature 'device-mapper' to 'devmapper'.

* Mon Nov 16 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.6-alt3
- initrd.mk: Disable IMAGEFILE and use default value.
- autodetect: Fix AUTODETECT variable.
- Rename INITRD -> RUN_INITRD.
- Turn off job control for emergency shell.
- Allow ROOT=/dev/nfs.

* Fri Nov 13 2009 Alexey Gladkov <legion@altlinux.ru> 0.1.6-alt2
- Add nfs subpackage.
- Add INITRD variable to identify initramfs.
- Minor bugfixes.

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
