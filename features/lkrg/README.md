# Feature: lkrg

Feature adds LKRG (Linux Kernel Runtime Guard) module loading.

The Linux Kernel Runtime Guard protects system by comparing hashes which are
calculated from the most important kernel region / sections / structures with
the internal database hashes. Additionally, special efforts have been made to
individually protect all extensions of the kernel (modules). To make the project
fully functional, the module should be initially loaded on a clean system.

https://lkrg.org

## Boot parameters

- `nolkrg` — do not load lkrg module during system boot.
- `noearlylkrg` — do not load lkrg module at initrd stage.
