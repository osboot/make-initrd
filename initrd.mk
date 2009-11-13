# save image into /tmp for tests.
#IMAGEFILE  = /tmp/initrd-$(KERNEL).img

# trying to detect modules for root and input devices(like keyboard)
AUTODETECT = root input

FEATURES = autodetect add-modules compress bootsplash cleanup
