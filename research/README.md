# Sanitized research data

`hitch-examples.csv` contains selected values from the most recent reviewed
private 0.5.1 log. `observations.json` records aggregate context and limitations.
Raw logs, process IDs, object addresses, local paths, captures and user media
are excluded. These values are observations, not code or assets from the game.

All examples were logged with the native 50,000 budget enabled and suballocation
off. They are from a mixed 87-second startup/load/fog-test session, not a labeled
ordinary-play traversal cohort. Approximate session seconds come from the preceding
queue summary, not exact per-call QPC. Individual call durations can cross intervals.

`call_thread_ms` sums completed tracked device calls on multiple threads.
`nested_kmt_thread_ms` is included in that duration. It records a broad CPU/GPU
synchronization wait, even though the original log calls it 'residency wait'.
`other_kmt_thread_ms` measures waits outside those calls. None establishes the
specific producer fence or the frame's critical path.

Examples include memory-related bursts, a long background-worker call, a DLSS-G
named worker burst, and intervals with almost no creation cost. Read
`docs/ARCHITECTURE.md` and `docs/EVIDENCE.md` before interpreting them.

Useful next research: bounded slow-call events with exact start/end QPC, API kind,
caller/resource class, thread, distinct Present streams, and the dependency being
waited on. Link paging/work fences before claiming a residency stall. Keep logging
overhead measurable. No diagnostic artifact in this repository enables suballocation.
