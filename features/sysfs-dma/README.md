# Feature: sysfs-dma

Feature adds autodetection support for DMA-related helper devices referenced
from sysfs.

It follows `dma:*` links below the detected device and recursively guesses the
dependencies of those DMA endpoints. This is mainly useful on platforms where
DMA wiring is exposed through auxiliary sysfs links rather than stable public
device nodes.
