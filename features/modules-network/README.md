# Feature: modules-network

Feature adds a collection of network modules available to the kernel.
These modules are not determined based on the current hardware.

## Parameters

- **GUESS_NET_IFACE** -- The list of network interfaces (`/sys/class/net/*`) for
  which you want to find modules. If the special word `all` is specified, then
  there will be a search for modules for all network interfaces.
