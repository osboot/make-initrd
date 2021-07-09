# Feature: smart-card

Feature adds smart card daemon and smart card utilities

## Configure

Use **SMART_CARD_PKCS11_MODULE** variable inside initrd.mk config to specify pkcs11 module to put. By default opensc-pkcs11.so module will be added.

For example:
```make
SMART_CARD_PKCS11_MODULE = librtpkcs11ecp.so
```
