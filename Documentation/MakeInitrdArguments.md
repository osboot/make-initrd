# Make-initrd arguments

There is a number of specific goals which are very useful in the generation of
the image.

- **guess-config** - This goal prints the intended configuration on stdout.
  It is compiled based on your current configuration and the feature list from
  `AUTODETECT` variable.
- **guess-modules** - This goal prints list of module needed to mount specified
  mountpoint or block device.
- **bug-report** - This goal collects system configuration data and puts those
  into an archive. If you have any problems with make-initrd this goal will help
  to gather the information necessary for developers to diagnose and fix bugs.

## See also

- The article about [configuration](Configuration.md).
- The list of [features](Features.md).
