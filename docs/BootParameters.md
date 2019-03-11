# Boot parameters

The following parameters can be passed via kernel command line.

#### Common parameteres

- `init=<path>` specifies the path to the init programm to be started after the initramfs has finished.
- `panic` enables/disables the receiving console in emergency situations.
- `rescue-modules` specifies the list of modules required to open emergency console.
- `stop=<comma-separated list>` specifies the list of stages to where it's necessary to open emergency console.
- `rdshell` opens emergency console after all services.
- `debug` shows more messages.
- `quiet` tries to be more quiet.
- `rd-preload-modules` enables/disables loading a static list of modules before udev service.

Parameters responsible for mounting the root file system:

- `root=<uuid|label|path|major:minor>` specifies the block device to use as the root filesystem.
- `rootdelay=<seconds>` specifies how long to wait for the root filesystem.
- `rootonly` ignore all mountpoints except root filesystem.
- `rootflags` specifies additional mount options.
- `rootfstype=<type>` specifies root filesystem type.
- `ro` mount root filesystem read-only.
- `rw` mount root filesystem read-write.

#### Resume

- `resume=<path|uuid|label>` resume from a swap partition.
- `resume_offset` offset in swap partition.
- `noresume` not resume and continue boot.

#### Network parameters

- `ip={on|any|dhcp|dhcp4|dhcp6}` get configuration from DHCP server for all interfaces.
  - `{dhcp4}` get only IPv4 configuration;
  - `{dhcp6}` get only IPv6 configuration;
  - `{dhcp}` try them both.
- `ip=<interface>:<autoconf>[:[<mtu>][:<macaddr>]]` describes the configuration for a specific interface. Optionally set `<macaddr>` on the `<interface>`.
- `ip=<ipaddr>:[<peer>]:<gw>:<netmask>:<hostname>:<interface>:<autoconf>[:[<mtu>][:<macaddr>]]` describes the network configuration explicitly. If you want do define a IPv6 address, put it in brackets (e.g. `[2001:DB8::1]`). The `<peer>` is optional and is the address of the remote endpoint for pointopoint interfaces and it may be followed by a slash and a decimal number, encoding the network prefix length. Optionally set `<macaddr>` on the `<interface>`. 
- `ip=<ipaddr>:[<peer>]:<gw>:<netmask>:<hostname>:<interface>:<autoconf>[:[<dns1>][:<dns2>]]` another way to describe the network configuration explicitly.
- `route=<net>/<netmask>:<gateway>[:<interface>]` adds a static route with route options, which are separated by a colon. IPv6 addresses have to be put in brackets. Example:
```
    route=192.168.200.0/24:192.168.100.222:eth0
    route=192.168.200.0/24:192.168.100.222
    route=192.168.200.0/24::eth0
    route=[2001:DB8:3::/8]:[2001:DB8:2::1]:eth0
```
- `nameserver=<IP>` specifies nameserver(s) to use.

These parameters can be specified multiple times.

#### NFS root

[See nfsroot feature](../features/nfsroot/README.md)

### LUKS

[See luks feature](../features/luks/README.md)

#### plymouth

[See plymouth feature](../features/plymouth/README.md)
