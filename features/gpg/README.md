# Feature: gpg

Feature adds GnuPG (The Universal Crypto Engine) and public keys to the image to
verify image signatures.

https://www.gnupg.org/software/index.html

## Parameters

- **GPG_PUBKEYS** -- List of files with public gpg keys.
- **GPG_PROG** -- The name of the gpg utility. This may be necessary if gpg is
  gpg-1.x and not gpg-2.x or higher.
