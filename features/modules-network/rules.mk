MODULES_TRY_ADD += \
        /drivers/net/phy/ \
        /drivers/net/team/ \
        /drivers/net/ethernet/ \
        ecb arc4 bridge stp llc ipv6 bonding 8021q af_packet virtio_net xennet

NETWORK_PATTERN_SET =
NETWORK_PATTERN_SET += symbol:eth_type_trans|register_virtio_device|usbnet_open
NETWORK_PATTERN_SET += not-filename:.*/wireless/.*
NETWORK_PATTERN_SET += not-filename:.*/isdn/.*
NETWORK_PATTERN_SET += not-filename:.*/uwb/.*
NETWORK_PATTERN_SET += not-filename:.*/net/ethernet/.*
NETWORK_PATTERN_SET += not-filename:.*/net/phy/.*
NETWORK_PATTERN_SET += not-filename:.*/net/team/.*
MODULES_PATTERN_SETS += NETWORK_PATTERN_SET
