# Boot order

- kernel calls `/init`. In the initramfs `/init` is the simplest script which saves the environment variables and calls `/sbin/init-bin`.
- `/sbin/init-bin` is SysV init which starts/stops services on runlevel 3.
- When the root file system found init goes to runlevel 2. Last service on runlevel 2 asks init to restart. When the SysV init receives request to reboot himself uses predefined path (`/sbin/init`).
- `/sbin/init` is another script which removes initramfs, restore kernel environment and runs real init.
