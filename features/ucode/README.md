# Feature: ucode

Feature adds CPU microcode loading at early stage before initramfs loading and
SMP init in kernel.  See
https://www.kernel.org/doc/Documentation/x86/early-microcode.txt for further
infomation how it works.

## Parameters

- **UCODE_CPU_VENDOR** -- Specifies the CPU vendor for which feature should add
  the microcode. Possible options: `AuthenticAMD` or `GenuineIntel`.
- **UCODE_CPU_FAMILY** -- Specifies the CPU family.
- **UCODE_ALL_MICROCODE** -- If defined `iucode_tool` will not select microcode
  by scanning online processors on this system for their signatures. All
  microcode for the processor vendor will be stored into image.
