IMAGEFILE  = /tmp/initrd-$(KERNEL).img
COMPRESS   = gzip
AUTODETECT = root net:wlan0

FEATURES = autodetect add-modules compress bootsplash lazy-cleanup
