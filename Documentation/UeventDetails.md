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
The daemon never starts two handlers for the same queue at the same time.
Different queues are independent and may run in parallel.

The queue name is therefore part of the synchronization model. Events that
operate on the same subsystem state should be put into the same queue. Events
that only need independent processing should use separate queues to avoid
blocking unrelated work.

`filters` should only classify incoming udev events and publish a small event
file. Heavy work, retries, device activation and mount attempts belong to
`handlers`. This keeps udev processing short and lets `ueventd` serialize the
real work for a queue.

Handlers come in two types:

1. `/lib/uevent/handlers/[0-9][0-9][0-9]-<QUEUE>`

This is an old version of `handlers`. Every handler is not called for every
event, but sequentially. A handler can analyze all the events that have arrived
and discard those if deemed necessary.

2. `/lib/uevent/handlers/<QUEUE>/[0-9][0-9][0-9]-<HANDLER>`

This is the new way to handle events. Handlers gets a directory with events
as an argument. Just like the first case, the handler is not called for
each event.

New code should prefer queue-specific handlers. They make the queue ownership
explicit and receive the directory with the current batch of events. This lets
the handler make one decision from the accumulated state instead of repeating
the same expensive operation for every event file.

Processed events are prefixed with `done.` or deleted.

## Queue Design Rules

Use one queue for events that must be processed sequentially. For example,
LVM activation, network root mounting, and NVMe over Fabrics autoconnect all
modify shared subsystem state and should not run several instances in parallel.

Use another queue when events can progress independently. This keeps a slow
handler from delaying unrelated subsystems.

Handlers should be idempotent. The same device state can be reported more than
once, several events can be batched together, and a handler may see that the
desired state was already reached by an earlier event.

Handlers should remove or mark only the events they have handled. Leaving an
event in place means the queue still has pending work and may be processed
again.

Delayed retries and timeout actions should go through the queue model too.
Feature handlers should not start independent background workers that perform
the same subsystem operation outside the queue, because that bypasses the
serialization guarantee. If a feature needs a delayed retry or a timeout
fallback, it should publish a later event to the same queue, preferably through
the common queue timer helpers.

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
