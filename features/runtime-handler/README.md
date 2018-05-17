# Feature: runtime-handler

Feature adds the ability to run in the `/initramfs` from real system after mounting the root filesystem. After `/initramfs` execution, the system will be rebooted.
It's **IMPORTANT** to remember, this script should take care itself of the remounting of the root filesystem in `RW` mode and also about its removal after the end of work.
