# How initramfs works ?

When we talk about event-driven infrastructure it means that there is no
hardcoded sequence of steps to mount system root.

The initramfs init starts a few services: udevd, [ueventd](UeventDetails.md),
[polld](PollDetails.md). The interaction between these services looks like this:

```
.==========.        .----------------.        .------------------.
||        ||        | polld          |     .->| boot real system |
|| kernel ||        |       .-----------.  |  '------------------'
||        ||        |       | extenders |--'
'=========='        |       |           |<-.
     ||             |       '-----------'  |
     ||             |                |     |
     ||             '----------------'     |
     ||  .----------. .--------------.     |
     |'->| devtmpfs | | mountpoints  |-----'
     |   | /dev     | | /...         |<----.
     V   '----------' '--------------'     |
.---------.        .-----------------.     |
| udevd   |        | ueventd         |     |
|  .---------.  .---------.  .----------.  |
|  | filters |->| uevents |->| handlers |--'
|  '---------'  '---------'  '----------'
|         |        |                 |
'---------'        '-----------------'
```

The `udevd` listens to kernel events and performs actions according to its rules.
Since we cannot run long-running processes or fork processes, we run special
`filters` (small simple scripts) to save event information for future use.
All events are added in one queue. The queue is implemented on the filesystem.
Fortunately, our filesystem is located in memory.

`ueventd` processes saved events. It's is a simple queue daemon. Each event in
the queue is processed one by one in the order of arrival, but daemon can
process several queues in parallel. It calls `handlers` for events in the
queue. A `handler` implements one aspect of boot (lvm, raid, luks, mount
device, etc). See [uevent details](UeventDetails.md) for more information.

For example, if we got events about all block devices needed for MD RAID then
the raid-handler will assemble it. This in turn will spawn a new kernel event
about the appearance of `/dev/md0`. Now, mount-handler can check it and if it
contains the root filesystem, then handler will mount this block device.

So, we mounted some block devices somewhere. We need something that would make
a decision that we met all the conditions for real system boot. The `polld` is
responsible for this. The server periodically runs `extenders` scripts that
perform checks (console is not active, the system init was found, etc.),
and if all scripts succeed, the server initiates a system boot. See [polling
details](PollDetails.md) for more information.

This scheme is used to mount system root device, resume system, configure
network. Basically it's used for everything based on kernel events.
