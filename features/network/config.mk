$(call feature-requires,depmod-image)

NETWORK_MODULES = ipv6 af_packet
NETWORK_DATADIR = $(FEATURESDIR)/network/data
NETWORK_PROGS   = ip udhcpc udhcpc6

GUESSv1_NETWORK_SOURCE := $(current-feature-name) net
GUESSv1_NETWORK_RULE0 = EMPTYVAR{GUESS_NET_IFACE}==false ACTION{RUN}=$(FEATURESDIR)/network/bin/guess
GUESSv1_NETWORK_RULE1 = VAR{FEATURES}+=network

