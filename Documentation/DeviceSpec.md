# Device specification

A device can be specified in several ways:

- `/<filename>` - The usual way to specify a device is to specify the device
  filename. You can specify the full path to the device file on which the root
  of the filesystem is located.

- `LABEL=<label>` - If the file system has a label, then you can define it
  through it. The value should begin with the `LABEL=` prefix. In this case,
  the `<label>` must be unique among block devices. Otherwise, a device that is
  initialized first will be used.

- `UUID=<uuid>` - For block devices with a filesystem, you can specify the UUID
  as the device identifier. The value should begin with the `UUID=` prefix.

- `PARTUUID=<uuid>` - The UUID may be either an EFI/GPT UUID, or refer to
   an MSDOS partition.

- `PARTLABEL=<label>` - The `<label>` being the GPT partition label. MSDOS
   partitions do not support labels!

- `SERIAL=<serial>` - The `<serial>` has the format: `<ID_MODEL>_<ID_SERIAL_SHORT>`
   It's possible to get these values using the `/lib/udev/ata_id`.

- `<major:<minor>` - you can specify device major and minor.

- `<HEX-number>` - major is (`number` / 256), minor is (`number` % 256).

- `/dev/disk/by-uuid/<UUID_ENC>`, `/dev/disk/by-label/<LABEL_ENC>`,
  `/dev/block/<major>:<minor>`, `/dev/disk/by-partuuid/<PARTUUID>`,
  `/dev/disk/by-partlabel/<PARTLABEL>` - There is also an alternative form for
  specifying the above methods.

If the specified value is not unique within the system, the result will not be
predictable.
