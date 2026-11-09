# BUDGET.md

Flash and RAM figures from the device build (`./build.sh`, the `ok image ...` line), one dated row per change that
moves them. Pool spare must never go under 2 KB; the target is 8 KB (`pool_spare` in `app/fm1.toml`). CPU figures
are not here until the device profiler has run on a real FM-1 (Phase 3).

All figures are from the host toolchain build, not measured on hardware.

| Date | Change | Image | SRAM used | RAM code | .data+.bss | Pool | Free (spare kept) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2026-11-09 | Untouched LOOM-8 v0.14 baseline | 579,716 B | 472,784 of 475,136 B | 20,172 B | 97,348 B | 355,256 B | 2,352 B (2,048 B) |
| 2026-11-09 | Rename to COLD (identity only, no code change that moves RAM) | 579,700 B | 472,784 of 475,136 B | 20,172 B | 97,348 B | 355,256 B | 2,352 B (2,048 B) |
