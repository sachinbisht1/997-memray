.. module:: memray

Memray API
==========

Memray exposes an API that can be used to programmatically activate or
deactivate tracking of a Python process's memory allocations. You do this by
creating a `Tracker` object and using it as a context manager in a ``with``
statement. While the body of the ``with`` statement runs, tracking will be
enabled, with output being sent to a destination you specify when creating the
`Tracker`. When the ``with`` block ends, tracking will be disabled and the
output will be flushed and closed.

API Usage
---------

.. autoclass:: memray.Tracker
   :members:

.. autoclass:: memray.FileDestination
   :members:

.. autoclass:: memray.SocketDestination
   :members:

.. autoclass:: memray.FileFormat()

   This enumeration lists the capture file formats that Memray can write. The
   `Tracker` constructor accepts a *file_format* keyword argument for choosing
   a different format than the default.

    .. autoattribute:: memray.FileFormat.ALL_ALLOCATIONS
       :annotation:

    Record every allocation that the tracked process performs. This is the
    default format. The produced capture files may be very large if the process
    performs many allocations. This is the only format that allows detecting
    :doc:`temporary allocations </temporary_allocations>` or using the
    :doc:`stats reporter <stats>`.

    .. autoattribute:: memray.FileFormat.AGGREGATED_ALLOCATIONS
       :annotation:

    For every location where the tracked process performed any allocations, the
    capture file includes a count of:

    - How many allocations at that location had not yet been deallocated when
      the process reached its heap memory high water mark
    - How many bytes had been allocated at that location and not yet
      deallocated when the process reached its heap memory high water mark
    - How many allocations at that location were leaked (i.e. not deallocated
      before tracking stopped)
    - How many bytes were leaked by allocations at that location

    You cannot find :doc:`temporary allocations </temporary_allocations>` using
    this capture file format, since finding temporary allocations requires
    knowing when each allocation was deallocated, and that information is lost
    by the aggregation. You also cannot use the :doc:`stats reporter <stats>`
    with this capture file format, because it needs to see every allocation's
    size to compute its statistics.

    Additionally, if the process is killed before tracking ends (for instance,
    by the Linux OOM killer), then no useful information is ever written to the
    capture file, because aggregation was still happening inside the process
    when it died.

    If you can live with these limitations, then ``AGGREGATED_ALLOCATIONS``
    results in much smaller capture files that can be used seamlessly with most
    reporters.

Temporal allocation records
---------------------------

The :meth:`memray.FileReader.get_temporal_allocation_records` method provides
access to the machine-readable allocation lifetime data used by Memray's
temporal flamegraph reporter.

It returns an iterable of :class:`memray.TemporalAllocationRecord` objects.
Each record describes allocations associated with an allocation site and
contains an ``intervals`` attribute containing :class:`memray.Interval` objects.

.. autoclass:: memray.FileReader
   :members: get_temporal_allocation_records

.. autoclass:: memray.TemporalAllocationRecord

.. autoclass:: memray.Interval

Temporal allocation intervals
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Each ``Interval`` describes the lifetime of one or more allocations between
memory snapshots.

The following fields are available:

* ``allocated_before_snapshot`` -- the index of the snapshot boundary before
  which the allocation was active.
* ``deallocated_before_snapshot`` -- the index of the snapshot boundary before
  which the allocation was deallocated, or ``None`` if the allocation remained
  live through the end of the captured data.
* ``n_allocations`` -- the number of allocations represented by the interval.
* ``n_bytes`` -- the total number of bytes represented by the interval.

The snapshot indices refer to the memory snapshots returned by
``get_memory_snapshots``.

Merging threads
~~~~~~~~~~~~~~~

:meth:`memray.FileReader.get_temporal_allocation_records` accepts a
``merge_threads`` argument. By default, ``merge_threads`` is ``True``, allowing
allocations from different threads to be represented together when they
otherwise belong to the same temporal allocation record.

Set ``merge_threads=False`` when thread-specific temporal allocation records
are required.

Open-ended lifetimes
~~~~~~~~~~~~~~~~~~~~

An interval with ``deallocated_before_snapshot`` set to ``None`` represents an
allocation that was not deallocated before the captured data ended.

These open-ended intervals are included in the temporal allocation records and
can therefore be distinguished from allocations that were explicitly
deallocated during the capture.

The temporal allocation records are the machine-readable representation used
by the temporal flamegraph reporter. The interval information therefore
corresponds to the allocation lifetime data consumed when generating a
temporal flamegraph.
