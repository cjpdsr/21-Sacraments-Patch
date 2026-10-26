# Repository packaging validation

- Imported the Nexus beta **0.5.2** source byte-for-byte and recorded file hashes.
- Verified the public RT worker has F8 polling and no F7 landmark handler.
- Verified the inherited configurable capture key remains forced empty.
- Verified suballocation and speculative prefetch remain disabled.
- Retained the upstream and MinHook licenses unchanged.
- Included sanitized observations, excluding raw logs, captures and game assets.
- Included a build wrapper and existing host-guard/budget-ownership test sources.

The repository preparation does not change C/C++ release behavior or the installed
private build. Python publication checks and PowerShell syntax checks are run
on the prepared files. No compiler was found during this packaging pass, so the
new root build wrapper was **not freshly compiled/executed** here. Historical
0.5.2 build/fixture results are separate in `release-0.5.2-validation.txt`.
No new gameplay or cross-game validation is claimed.
