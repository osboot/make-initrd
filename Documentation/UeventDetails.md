# Ueventd Details

`ueventd` is a queue daemon. The task of this server is to process events
from the queue. Each event in its queue is processed one by one in the order
of arrival, but several queues can be processed in parallel.

Queues are implemented on the filesystem. All incoming events are placed as:

`/.initrd/uevent/queues/<QUEUE>/<EVENT>`

Events come to this directory all the time. As soon as the server receives
a inotify event that something appeared in the directory, it moves all files
from the queue directory to:

`/.initrd/uevent/events/<QUEUE>/<EVENT>`

This is done to avoid a race condition between `filters` that add new events and
`handlers` that process them slowly. All handlers are called one by one.
Handlers come in two types:

1. `/lib/uevent/handlers/[0-9][0-9][0-9]-<QUEUE>`

This is an old version of `handlers`. Every handler is not called for every
event, but sequentially. A handler can analyze all the events that have arrived
and discard those if deemed necessary.

2. `/lib/uevent/handlers/<QUEUE>/[0-9][0-9][0-9]-<HANDLER>`

This is the new way to handle events. Handlers gets a directory with events
as an argument. Just like the first case, the handler is not called for
each event.

Processed events are prefixed with `done.` or deleted.

## Uevent API

All `filters` and `handlers` should use `/bin/uevent-sh-functions` as
the interface to events.

`make_event` is used to create a new event. It will not be published yet.
The `<QUEUE>` is passed as an argument. This function returns the eventfile.
You can write any necessary content to this file.

`publish_event`, `release_event` publishes an event. After that call, the event
becomes available for processing.

`done_event` marks eventfile as processed. Processed events are prefixed with
`done.` or deleted.

## See also

- The article about [runtime](BootInitramfs.md).
