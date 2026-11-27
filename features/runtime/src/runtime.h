// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MKRD_RUNTIME_H
#define MKRD_RUNTIME_H

int chrooted_main(int argc, char **argv);
int environ_main(int argc, char **argv);
int halt_main(int argc, char **argv);
int nfsmount_main(int argc, char **argv);
int resume_main(int argc, char **argv);
int runas_main(int argc, char **argv);
int sysexec_main(int argc, char **argv);
int timestamp_main(int argc, char **argv);
int ueventd_main(int argc, char **argv);
int ueventd_mounts_main(int argc, char **argv);

#endif /* MKRD_RUNTIME_H */
