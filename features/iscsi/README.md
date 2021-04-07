# Feature: iscsi-boot

Feature adds you to perform a diskless system boot using pxe and iSCSI.

## Parameters
- **ISCSI_TARGET_NAME** -- Target name iSCSI, required parameter
- **ISCSI_TARGET_IP** -- Target IP address iSCSI, required parameter 
- **ISCSI_TARGET_PORT** -- Target port iSCSI 
- **ISCSI_TARGET_LUN** -- Target group tag iSCSI
- **ISCSI_INITIATOR** -- Initiator name iSCSI
- **ISCSI_USERNAME** -- Username for initiator authentication by the target(s)
- **ISCSI_PASSWORD** -- Password for initiator authentication by the target(s) 
- **ISCSI_IN_USERNAME** -- Username for target(s) authentication by the initiator
- **ISCSI_IN_PASSWORD** -- Password for target(s) authentication by the initiator
