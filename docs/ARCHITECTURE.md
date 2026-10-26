# Active code and retained research code

`src/silenttraverse.cpp` derives D3D12 interception, accounting, configuration,
logging, and hook infrastructure from Ishimura Patch. `src/vtindex.c` supplies
interface slots using the SDK declarations. MinHook is vendored separately.

The active performance adjustment is in `src/rt_budget.h`. It verifies the host,
registration and scheduling fingerprints, resolves the expected backing integer,
checks memory access, and applies a 50,000 primitive budget. It does not invoke
the UE console-variable setter or claim its setter semantics.

`src/budget_override.h` owns the conditional write: capture the previous value,
compare/exchange the override, restore only if the field still equals the applied
value, and stop owning it when another writer changes it. The worker does not
continually fight the engine or another mod. F8 changes the enabled state and
persists the preference. Public 0.5.2 has no F7 handler.

The executable SHA256 guard runs before configuration I/O and hook initialization.
SH2-specific registration/scheduler fingerprints are checked again before writes.
Offsets and fingerprints in this code describe one tested game build; they are
not a generic Unreal Engine interface.

## Disabled paths

Shared-heap conversion is forced off by configuration and `SubTry` returns false
before conversion. `SubPrewarm` returns before creating pools. `Lookahead::enabled`
is false and the speculative prefetch worker is not started. Frostbite-specific
engine-field scanning and setting changes are not part of the active SH2 fix.

Large inherited allocator and prefetch implementations remain in the snapshot
for provenance and engineering inspection. Their presence is not evidence of an
adapted or validated UE allocator. Removing a return or changing a default does
not implement residency integration. See `PORTING.md`.

## Known diagnostic limitations

The inherited setup banner saying fresh committed PREFETCH is active is stale
in this source. The explicit RT_BUDGET mode and disabled code paths are authoritative.
The repository preserves the shipped source; it does not silently fix the banner.

The log's 'residency wait' field measures nested
`D3DKMTWaitForSynchronizationObjectFromCpu` time. The hook does not decode which
fence or dependency was waited on. It is not proof of memory paging. Completed
device calls contribute their entire durations to the next drained frame counter,
even if those calls started in an earlier frame. Thread sums can overlap.
Hooked Present streams are not separated by swapchain in the summary.
