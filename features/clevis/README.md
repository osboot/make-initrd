# clevis

The `clevis` feature adds optional support for Clevis-based LUKS unlock in
initrd.

When this feature is enabled, the `luks` handler tries to unlock a LUKS device
with Clevis before falling back to the existing interactive passphrase prompt.

If Clevis is not available in the image, or if Clevis-based unlock fails, the
existing password prompt logic is preserved.

## Requirements

This feature is intended to be used together with the `luks` feature.

A Clevis-enabled LUKS device must be provisioned in advance on the installed
system, for example with TPM2 binding.

For more information about Clevis see:
<https://github.com/latchset/clevis>

## Configuration

Enable the feature by adding it to `FEATURES`:

```make
FEATURES += clevis
```
