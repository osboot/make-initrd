# Guess module: loaded-modules

This feature tries to take into account loaded modules for some architectures.
For some single-board computers, it is very difficult to determine the
dependencies between some of the required modules.

There is a module-sbc feature to add all modules that may be needed in this
case, but this approach is not always applicable.

Therefore, an intermediate approach is taken with the inspection of the loaded
modules.

You can disable this feature by adding to the `/etc/initrd.mk`:
```
DISABLE_GUESS += loaded-modules
```
