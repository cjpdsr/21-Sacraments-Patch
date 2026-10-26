# 21 Sacraments Patch

Experimental native traversal-stutter mitigation for **Silent Hill 2 Remake**.
Created by **bigbullet3**. **Based on Ishimura Patch by lukeman3000**:
https://github.com/lukeman3000/ishimura-patch

This repository contains the **public Nexus beta 0.5.2 source**, with **F8 toggle only**.
The private tester build's F7 landmark handler is absent. Legacy configurable
capture support remains in the inherited source but is forced off in this port.

## What works today

The patch changes the verified native backing integer associated with
`r.RayTracing.Geometry.MaxBuiltPrimitivesPerFrame` to a **50,000 primitive soft budget**.
This limits eligible queued ray-tracing geometry build work scheduled together.
Immediate/forced requests and individual large builds can exceed the budget.
Disabling restores the captured original value only while the patch still owns it.

**Shared-heap suballocation and speculative allocation prefetch are disabled.**
The observed improvement is from RT geometry build scheduling. This does not
increase the available VRAM pool, fix every UE5 hitch, or establish a universal
budget for other hardware or games.

## For players

Use the existing **21 Sacraments Patch Nexus release** for the packaged installer.
This source archive contains no ready-to-install loader, ASI, PAK, or game files.
F8 toggles the working fix with rising/falling tones; the fresh configuration
starts enabled after initialization. F7 has no public-release action.

Only the tested Windows x64 Steam executable is accepted:
`c21ae87cdd186ca22760ad7aa66973eaac2a4d9a4e437a3b42daaab08eb729bc`.
Unknown executables are refused before mod configuration I/O or hook installation.

## For developers

Read [the architecture](docs/ARCHITECTURE.md), [build instructions](docs/BUILD.md),
[porting guide](docs/PORTING.md), and [research findings](research/README.md).
The SH2-specific addresses, signatures, executable hash, and budget must be
re-established for each target. Copying this ASI to another game is not a port.

```powershell
.\build.ps1 -Zig "C:\tools\zig\zig.exe"
```

Use Zig **0.13.0** on Windows x64. The script builds into `build/`, runs the
unsupported-host guard and budget-ownership fixture, and never installs files.
Python 3 can independently check the published source snapshot:

```text
python tools/verify_repository.py
```

## Evidence and open questions

The original tester reported smoother traversal, and Wccftech independently
reported improved low-percentile frame rates on its own SH2 route. Their results
are external observations, not a bundled benchmark or a guarantee for every system.
See [evidence and limitations](docs/EVIDENCE.md).

Sanitized event examples and aggregate findings are included under `research/`.
Remaining spikes show mixed creation and synchronization activity. The summary
logger cannot identify exact fence dependencies or prove the critical path.
Suballocation is a research question, not an enabled second optimization.

## Reuse and credits

Read [the original-contributions terms](CONTRIBUTIONS-LICENSE.txt),
[Luke's unchanged license](LICENSE), [license scope](docs/LICENSE-SCOPE.md),
and [third-party notices](THIRD_PARTY_NOTICES.md).

For original contributions covered by bigbullet3's terms, private noncommercial
study, builds and modification are allowed. **External redistribution, including
modified releases or ports containing those contributions, and commercial use
require prior written permission.** Approved reuse must include credit and the
required notices. GitHub's own platform permissions remain applicable.

Luke's license continues to govern the inherited Ishimura material; its permission
to share qualifying forks does not itself grant rights to the separately covered
original contributions. Third-party licenses are unchanged. These new terms do
not revoke permissions already granted for earlier copies. The project is
**source-available**, not MIT-licensed or unrestricted.

If you adapt this work, credit **Ishimura Patch by lukeman3000** with its repository
link and identify the SH2 adaptation by **bigbullet3**. This is an independent
project, not an official Ishimura Patch release.

No game assets, extracted shaders, GPU capture files, personal logs, save files,
or private F7 build are included. Contributions should follow
[CONTRIBUTING.md](CONTRIBUTING.md).
