# Contributing

Bug reports should include game build/hash, patch version, GPU/VRAM, driver,
CPU/RAM, graphics settings, upscaler, frame generation, frame cap/VSync, and a
location or route. Explain whether the issue happens with the budget enabled,
disabled, on a repeat pass, and with other modifications present.

For a new-game port, document the target's control path and evidence; do not
copy SH2 fingerprints. For performance changes, attach comparable before/after
measurements and note logging overhead. Include negative results and graphical
side effects. Keep independent variables separate.

For allocator work, start with a design explaining pageable-object ownership,
residency accounting, queue completion, reuse and aliasing. Removing the disabled
path's gates alone is not an accepted allocator implementation.

Do not submit game executables/assets, extracted proprietary shaders, private
save files, credentials, full system traces or unredacted local paths. Share
minimal relevant evidence that you have permission to distribute.

Preserve upstream attribution and licenses. Read `CONTRIBUTIONS-LICENSE.txt` and
`docs/LICENSE-SCOPE.md` before proposing code or a distributable port. State the
license and ownership of your own submissions explicitly; submitting a patch
does not transfer your ownership to bigbullet3. Do not assume permission to
redistribute a modified release or use covered original contributions commercially.
