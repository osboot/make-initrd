# Feature: rdshell

Feature gives the ability to get shell in initrd. It also allows you to set a password
for shell inside initrd or even completely disable it.

## Parameters

- **RDSHELL** -- The parameter sets the mode of the shell. Available options:
  - **shell** -- provides shell without restrictions;
  - **login** -- restricts access to shell by password;
  - **disable** -- disables the ability to use shell in initrd.
- **RDSHELL_PASSWORD** -- Encrypted password (see crypt(3) for details). This option implies **RDSHELL=login**.
- **RDSHELL_COPY_PASSWORD_FROM_USER** -- The parameter contains the name system user which password will be used to restrict access to shell inside initrd. This option implies **RDSHELL=login**.
