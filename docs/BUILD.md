# Building the public source

Use Windows x64 and Zig 0.13.0 with its bundled Clang/MinGW headers and libraries.
The original release was built with that version. No terminal compiler is installed
by these scripts. The build has no automatic downloads and requires no game files.

From the repository folder in PowerShell:

```powershell
.\build.ps1 -Zig "C:\tools\zig\zig.exe"
```

Outputs are `build/silenttraverse.asi`, `build/host_guard_test.exe`,
`build/budget_test.exe`, and `build/test-results.txt`. Both fixtures run from
`build/`. The guard deliberately loads the ASI into an unsupported test process
and expects refusal before configuration I/O or game hooks. The ownership fixture
checks conditional writes/restoration and coexistence with another value writer.
These fixtures do not establish GPU correctness or full-game stability.

The original packaged build script remains in `src/build.ps1` as part of the
unchanged source snapshot. Prefer the root script for an isolated output folder.
`src/build_id.h` preserves the public release identity; compiler dates/times mean
a local rebuild is not claimed to be byte-identical to the published ASI.

The source already vendors MinHook. An ASI loader is a separate dependency;
the player's package used Ultimate ASI Loader by ThirteenAG. This source tree
does not bundle that binary or recreate the Nexus installer payload. Follow the
loader's own documentation and license when assembling a distribution.

Nothing in the build script copies into Steam, changes installed settings, opens
the game, or publishes a release. Existing game installations are independent.

Before changes, run `python tools/verify_repository.py` to confirm the imported
0.5.2 source. After intentional changes, its snapshot checks will report a diff;
that is expected and should be reviewed rather than hidden. Do not disable the
runtime game fingerprint gate simply to get an experimental build to load.

Packaging validation for this repository is recorded in
`docs/REPOSITORY-VALIDATION.md`. Historical release fixture results are retained
in `docs/release-0.5.2-validation.txt`; they are not a fresh test of new changes.
