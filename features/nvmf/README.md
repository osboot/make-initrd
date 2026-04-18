# Feature: nvmf

Feature adds support for booting from NVMe over Fabrics devices.

## Parameters

- **NVMF_HOSTNQN** -- NVMe host NQN.
- **NVMF_HOSTID** -- NVMe host ID.
- **NVMF_CONNECT** -- NVMe connection entry in the form
  `<transport>,<traddr>,<subnqn>[,<trsvcid>]`.
- **NVMF_DISCOVER** -- NVMe discovery entry in the form
  `<transport>,<traddr>[,<trsvcid>]`.

Only TCP entries are supported initially.
