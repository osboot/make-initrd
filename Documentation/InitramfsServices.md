# Initramfs services

The initramfs uses SysV-style init scripts. They are [LSB compilant](http://refspecs.linuxfoundation.org/LSB_5.0.0/LSB-Core-generic/LSB-Core-generic/initscrcomconv.html).

## LSB Header

The init script must include a special comment header providing some meta information about the init script.
The LSB Header is bounded by comments, specifically, the beginning of the header is marked with:
```
### BEGIN INIT INFO
```
The end of the LSB Header is marked with:
```
### END INIT INFO
```

The LSB Header is composed of the following sections:

### Provides
```
# Provides: boot_facility_1 [boot_facility_2...]
```
The LSB Header lists any boot facilities that this service provides. Other services can reference these boot facilities
in their `Required-Start` and `Required-Stop` headers. It is usually the name of the daemon. If more than one package
can provide the same facility.

### Required-Start
```
# Required-Start: boot_facility_1 [boot_facility_2...]
```
The LSB Header lists any boot facilities which must be available during startup of this service.

### Required-Stop
```
# Required-Stop: boot_facility_1 [boot_facility_2...]
```
The LSB Header lists any boot facilities which should *NOT* be stopped before shutting down this service.

### Should-Start
```
# Should-Start: boot_facility_1 [boot_facility_2...]
```
The LSB Header lists any facilities, which, if present, should be available during startup of this service.
The intent is to allow for weak dependencies which do not cause the service to fail if a facility is not available.

### Should-Stop
```
# Should-Stop: boot_facility_1 [boot_facility_2...]
```
The LSB Header lists any facilities, which, if present, should only be stopped after shutting down this service.
The intent is to allow for weak dependencies which do not cause the service to fail if a facility is not available.

### X-Start-Before
```
# X-Start-Before: boot_facility_1 [boot_facility_2...]
```
The LSB Header implies that the script using this keyword should be started before the specified facilities.

### Default-Start, Default-Stop
```
# Default-Start: run_level_1 [run_level_2...]
# Default-Stop: run_level_1 [run_level_2...]
```
The LSB Header lists the runlevels for which the service will be enabled (disabled) by default. These runlevels are space-separated.

### Short-Description
The LSB Header provides a brief summary of the actions of the init script. This must be no longer than a single, 80 character line of text.

### Description
The LSB Header provides a more complete description of the actions of the init script.  May span mulitple lines.
In a multiline description, each continuation line shall begin with a '#' followed by tab character or a '#'
followed by at least two space characters. The multiline description is terminated by the first line that
does not match this criteria.
