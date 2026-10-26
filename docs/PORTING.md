# Adapting the research to another game

The reusable result is an approach: locate bursts of RT geometry build work,
check whether the engine already has a scheduling budget, and test a supported
way to spread eligible work while retaining correct graphics behavior.

1. Establish an application-frame baseline on a repeatable traversal. Record
   exact build, hardware, settings, upscaler, frame generation, cap and VSync.
2. Identify the engine scheduling path for that build. Determine which builds
   are queued, immediate, forced, oversized, or updated, and which work bypasses
   the budget. Do not infer behavior solely from a variable's name.
3. Prefer a supported control when available. If a native override is needed,
   re-establish the data representation, lifetime, exact host identity, code/data
   fingerprints, thread interaction, and conditional restoration. SH2 RVAs do
   not transfer, even to another UE build.
4. Measure the same route under comparable conditions, including repeat runs and
   warm/cold behavior. Inspect geometry, reflections, lighting, and scene transitions
   for effects of delayed work. Report frame-time tails and individual spikes.
5. Attribute remaining stalls before choosing another optimization. A driver
   stack using a CPU heap is not proof of VRAM allocation pressure, and a short
   MakeResident API call does not establish fast asynchronous completion.

## Why shared-heap suballocation is a separate project

A committed resource owns an implicit heap. Replacing it with a placed resource
moves physical memory ownership to an explicit shared heap. UE's residency manager
must then account for the right pageable object, aggregate shared usage, handle
eviction and budget changes, and preserve queue/fence/lifetime guarantees.

Before implementing that, demonstrate costly eligible creation on the affected
frame's critical path. Establish resource/caller classes, heap compatibility,
alignment and resource flags, residency ownership, safe reuse after all relevant
queues finish, aliasing barriers where needed, heap retirement and device removal.
Measure CPU bookkeeping, reserved memory and VRAM pressure as well as frame pacing.
UE already emits many placed-resource calls in the observed SH2 sessions.

The shipped SH2 adaptation does not implement this integration. A second game
using the same graphics API can require different ownership and scheduling work.
The separate percentages reported for Dead Space and SH2 do not define a target
that an allocator change is expected to reach.
