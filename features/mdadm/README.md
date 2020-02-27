# Feature: mdadm

Feature adds the ability to boot from RAID using mdadm.

## Boot parameters

- `raid-member-delay=<SECS>` - determines the number of seconds to wait after
  the last raid member appears before starting the degraded raid (default: 10
  seconds). If this value is zero, it means disabling the start of a degraded
  raid.

