# Feature: lkrg

Feature adds p_lkrg (Linux Kernel Runtime Guard loadable module) module loading.

The Linux Kernel Runtime Guard protects system by comparing hashes which are
calculated from the most important kernel region / sections / structures with
the internal database hashes. Additionally, special efforts have been made to
individually protect all extensions of the kernel (modules). To make the project
fully functional, the module should be initially loaded on a clean system.

https://openwall.info/wiki/p_lkrg/Main
