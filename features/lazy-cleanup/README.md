# Feature: lazy-cleanup

Feature changes the image build flow to clean the output directory before
creating a new image.

It is a build-time helper only and does not add runtime files to initramfs.
