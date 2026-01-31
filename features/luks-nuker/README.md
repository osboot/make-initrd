# Feature: luks-nuker

Feature provides support for emergency destruction of encrypted partitions.

## About

The luks-nuker precedes cryptsetup's input, read it and tries to match it
with the nuking password. Under normal circumstances it doesn't reveal
itself at all.

Refer to the `/etc/luks-nuker.conf` to get more information about and
configure it.

## How to get

You can get luks-nuker from [git
repository](http://git.altlinux.org/people/ved/public/luks-nuker.git).
Also it is distributed by ALT Linux Sisyphus.

## Parameters

- **LUKS_NUKER_ARGS** -- Specifies luks-nuker arguments. Empty by default.
