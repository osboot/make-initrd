# Guess module: net

This modules adds kernel modules for network devices (netboot).

## Parameters

- GUESS_NET_IFACE -- The list of network interfaces (`/sys/class/net/*`) for
  which you want to find modules. If the special word `all` is specified, then
  there will be a search for modules for all network interfaces.
