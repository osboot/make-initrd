# Polld Details

`polld` it is a daemon for periodically executing scripts. All scripts are
executed one by one. The daemon executes two types of scripts:

- Boot contidion [scripts](PollDetails.md#boot-conditions);
- Other custom [scripts](PollDetails.md#other-scripts).

## Boot Conditions

The result of the initramfs can not only search and boot from the root partition.
In some cases it may be necessary to wait for certain conditions and do a kexec.

The various sets of conditions to expect during initramfs execution is called
the boot methods. A separate directory is created for each method:

`/lib/initrd/boot/method/<METHOD>`

From this directory `polld` uses two scripts "check" and "action". The "action"
script will be executed if "check" succeeds.

On each iteration, `polld` reads the name of the current boot method from the
`/etc/initrd/method` file. So, at some point, one boot method may switch to
another method.

The default initramfs uses the "localdev" boot method. This method will boot to
the new root partition if:

- resume failed.
- bash (emergency shell) is not running on the console.
- all required mountpoints are ready and mounted.
- system init found.

But other boot methods may have their own checks and actions.

## Other Scripts

Scripts from the directory are executed sequentially one by one.

`/lib/uevent/extenders/`

Scripts in this directory must be prefixed with `[0-9][0-9][0-9]-`. The daemon
ignores the return code of scripts.
