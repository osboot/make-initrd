$(call feature-requires,depmod-image)

NETWORK_MODULES = ipv6 af_packet
NETWORK_DATADIR = $(FEATURESDIR)/network/data
NETWORK_PROGS   = ip udhcpc udhcpc6
