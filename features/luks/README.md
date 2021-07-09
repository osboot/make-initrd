# Feature: luks

Feature adds the ability to boot from LUKS partition.

## Boot parameters

- `no-luks` disables crypto LUKS detection.

- `luks-key=`

  `<keypath>[:<keydev>][:<luksdev>]` key for luks device on removable device

  `pkcs11:<pkcs11-path>[:<luksdev>]` key for luks device on smart-card device
  - `keypath` is a path to key file to look for.
  - `keydev` is a device on which key file resides (see [device spec](../../Documentation/DeviceSpec.md)).
  - If `luksdev` is given, the specified key will only be applied for that LUKS device.
    Possible values are the same as for keydev. Unless you have several LUKS devices,
    you don’t have to specify this parameter.
  - `pkcs11-path` is a path to data object on pkcs11 device in format: [serial=<serial>];id=<id>|label=<label>
    - `serial` is smart card serial number
    - `id` is id of data object
    - `label` is application label of data object

- `luks-key-format=<format>` defines the format of the key file
  (possible values: `plain`, `raw`)

- `luks-discard` is an array of luks devices specified in same format in luksdev
  to which discard operation is perfomed (is essential for SSD).

- `luks-ignore` specifies which luks device should be ignored by initrd to decrypt.

- `luks-dev` - the parameter allows you to specify a device if there are more
  than one partitions in the luks system (see [device spec](../../Documentation/DeviceSpec.md)).

## Configuration file

You can put the `/etc/luks.keys` file in initramfs and then you do not need to
specify boot options. The file describes which keys for which LUKS partitions to use
and where to find them.

The file contains entries separated by a _single_ tab character. Each entry
describes one key file.

```
key-path[<TAB>key-device[<TAB>luks-device]]
```

The `key-device` and `luks-device` fields are optional. If only the `key-path`
is specified, then the key will be used to all found LUKS partitions. If the key
is on a separate device (`key-device`), then `key-path` must be specified from
the root of this device.

## Examples

Luks key on device:
```
luks-key=keys/luks.key:UUID='eee52cfb-4029-423b-8736-b494a252c387'
```

Luks key on the smart card:
```
luks-key=pkcs11:label=luks-key luks-key-format=raw
```
