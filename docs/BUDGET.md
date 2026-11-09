# BUDGET.md

Flash and RAM figures from the device build (`./build.sh`, the `ok image ...` line), one dated row per change that
moves them. Pool spare must never go under 2 KB; the target is 8 KB (`pool_spare` in `app/fm1.toml`). CPU figures
are not here until the device profiler has run on a real FM-1 (Phase 3).

All figures are from the host toolchain build, not measured on hardware.

| Date | Change | Image | SRAM used | RAM code | .data+.bss | Pool | Free (spare kept) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 2026-11-09 | Untouched LOOM-8 v0.14 baseline | 579,716 B | 472,784 of 475,136 B | 20,172 B | 97,348 B | 355,256 B | 2,352 B (2,048 B) |
| 2026-11-09 | Rename to COLD (identity only, no code change that moves RAM) | 579,700 B | 472,784 of 475,136 B | 20,172 B | 97,348 B | 355,256 B | 2,352 B (2,048 B) |
| 2026-11-09 | id_prefix FM-1_44 (identity only) | 579,700 B | 472,784 of 475,136 B | 20,172 B | 97,348 B | 355,256 B | 2,352 B (2,048 B) |
| 2026-11-09 | Phase 1a: weave and conductor hidden in the UI (code still there) | 579,664 B | 472,768 of 475,136 B | 20,172 B | 97,332 B | 355,256 B | 2,368 B (2,048 B) |
| 2026-11-09 | Phase 1a: weave removed (engine, data, UI), PROJ_FORMAT 21, old formats dropped | 569,616 B | 468,944 of 475,136 B | 20,172 B | 96,364 B | 352,400 B | 6,192 B (2,048 B) |
| 2026-11-09 | Engine picker offers only kept engines; new-project defaults use rayon and jacquard (no code removed yet) | 569,552 B | 468,944 of 475,136 B | 20,172 B | 96,364 B | 352,400 B | 6,192 B (2,048 B) |
| 2026-11-09 | Phase 1c: NTRK 8 to 5, PROJ_FORMAT 22, USB audio 8 channels | 568,108 B | 439,364 of 475,136 B | 20,172 B | 91,748 B | 327,436 B | 35,772 B (2,048 B) |
| 2026-11-09 | Phase 1b: knit, bellows, cane, voile, acid, slice removed (code, presets, tests) | 505,336 B | 419,724 of 475,136 B | 14,156 B | 90,416 B | 315,148 B | 55,412 B (2,048 B) |
