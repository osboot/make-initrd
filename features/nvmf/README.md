# Feature: nvmf

Feature adds support for booting from NVMe over Fabrics devices.

The guess stage enables this feature automatically for NVMe controllers with
`tcp`, `rdma`, or `fc` transports.  Local PCIe NVMe subsystem links are followed
without enabling this feature.

## Parameters

- **NVMF_HOSTNQN** -- NVMe host NQN.
- **NVMF_HOSTID** -- NVMe host ID.
- **NVMF_NONVMF** -- disable NVMe over Fabrics setup.
- **NVMF_NOSTATIC** -- ignore static `/etc/nvme/discovery.conf` and
  `/etc/nvme/config.json` configuration copied into the image.
- **NVMF_NONBFT** -- ignore ACPI NBFT information.
- **NVMF_CONNECT** -- NVMe connection entry in the form
  `<transport>,<traddr>,<subnqn>[,<trsvcid>[,<host-traddr>]]`.
- **NVMF_DISCOVER** -- NVMe discovery entry in the form
  `<transport>,<traddr>[,<host-traddr>[,<trsvcid>]]`.

The `tcp`, `rdma`, and `fc` transports are supported.  For compatibility with
earlier make-initrd images, `NVMF_DISCOVER=<transport>,<traddr>,<trsvcid>` is
still accepted when the third field is numeric.  Fibre Channel discovery also
accepts `NVMF_DISCOVER=fc,auto`.

The lowercase dracut-style aliases are accepted on the kernel command line:
`rd.nonvmf`, `rd.nvmf.nostatic`, `rd.nvmf.nonbft`, `rd.nvmf.hostnqn`,
`rd.nvmf.hostid`, `rd.nvmf.connect`, and `rd.nvmf.discover`.
