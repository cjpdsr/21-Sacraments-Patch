# Evidence, scope, and unknowns

## Working public version

Public Nexus beta 0.5.2 uses the same 50,000 primitive soft budget as the earlier
working private version. Its public control is F8; F7 landmark polling was removed.
The original tester reported a substantial improvement on the Nathan Avenue
town route. Full-story stability and all hardware combinations were not established.

Wccftech independently reported improved low-percentile performance on its own
approximately 40-second route with an RTX 4090 and i7-14700K. Its article is an
external benchmark, not a result produced by this repository:
https://wccftech.com/silent-hill-2-remakes-infamous-traversal-stutters-significantly-reduced-by-new-mod-our-tests-show-52-higher-0-1-lows/

Keep hardware, test route, baseline and methodology attached to any percentage.
Results from Dead Space with Ishimura Patch are not a directly comparable score
for the SH2 implementation and do not predict benefits of enabling another path.

## Remaining stutters

`research/` contains selected anonymous event examples and aggregate observations
from a short private 0.5.1 test session using the same budget adjustment. That
session included startup, loading and a fog test. It was not a controlled traversal
benchmark of public 0.5.2. The examples support several hypotheses, not one proven
root cause or an expected suballocation speedup.

Some long Present intervals coincide with substantial creation activity. Others
coincide with tiny creation cost and waits outside creation. Sparse VRAM snapshots
were below budget. The logger cannot identify fence ownership, async residency
completion, exact interval overlap, GPU execution duration or all Present streams.

Do not add thread sums as though they were serial frame costs, subtract them to
predict an optimized frame, or count nested waits twice. A call completing during
one frame interval can include time from earlier frames.

## Not established

- Shared-heap conversion correctness under UE's residency manager.
- That creation or residency dominates the remaining rare gameplay hitches.
- A portable optimal primitive budget or universal UE5 fix.
- Compatibility with unknown game hashes or every mod/overlay/driver combination.
- A fix for fog ghosting. The separate fog experiments are outside this release.
