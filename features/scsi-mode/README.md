# Feature: scsi-mode

Feature adds autodetection support for devices operating in SCSI disk mode.

If the detected sysfs device exposes a `scsi:` modalias, the feature requests
the `sd_mod` kernel module.
