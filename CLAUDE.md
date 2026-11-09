# CLAUDE.md

Custom firmware for the **M-VAVE FM-1**: a minimal 4-track groovebox. It is a **fork of LOOM-8 v0.14**
(charlesvestal/fm1-loom), cut down, with **FMini-1's synth engine** (ruppert/fmini-1 v2.5.0) added and a new
sequencer UI designed by Kad. The project is called **COLD** (repo `COLD`). Build file names use lowercase
`cold` (`app/build/cold.fwsc`).

The step-by-step plan is in [docs/PLAN.md](docs/PLAN.md). Read it at the start of every session and continue from
the first unchecked item. LOOM-8's own docs stay the reference for what is already here: `BUILDING.md`,
`LICENSING.md`, `docs/MANUAL.md`, `docs/ux-review.md`, `docs/superpowers/specs/`.

## Working with Kad

- Kad is a design engineer (frontend and design background). He owns the sequencer and UI design. Explain embedded
  or DSP points briefly when a decision depends on them.
- Kad flashes the hardware himself. **Never flash, install, back up or restore an FM-1 unless Kad asks in that
  session.** Prepare the package and the exact command, then stop.
- Never push, tag or publish a release unless asked. Commit on a branch per phase (`phase-1-cut`, ...).
- Docs, commit messages and notes for Kad: plain English, no em dashes, no all-caps emphasis.
- Nothing is tested on a real FM-1 until Kad says so. Say "host-tested only" in any summary of firmware changes.
- When a choice is Kad's (sound, UI, what to cut), lay out the options with their RAM/CPU cost and ask.

## Decisions already made (do not reopen without Kad)

| Area | Decision |
| --- | --- |
| Base | Fork LOOM-8 v0.14 and subtract. Keep fm1-base and felucca-bsp as the `base/` submodule. |
| Tracks | 4: 3 synth + 1 drum. |
| Main synth | FMini-1's engine, mono or poly capped at 4 voices, with its 200 factory patches. |
| Other synths | 1 or 2 of LOOM-8's own engines (Kad picks; knit is the default candidate). The rest are cut. |
| Engine choice | Any synth track can pick any synth engine (FMini or a kept LOOM-8 engine). The drum track plays the kit. |
| Connectivity | Keep Bluetooth MIDI (fm1-base's own stack) and screen mirroring. |
| Drums | LOOM-8's 11-sound drum kit, unchanged at first. |
| FX | LOOM-8's delay and reverb sends, master compressor and limiter. Chorus, lofi, phaser are cut candidates. |
| Sequencer data | Keep LOOM-8's pattern model: steps, p-lock lanes, trig conditions (skip cycles, flam, glide, bend, jump), scenes, song. Cut weave, conductor ("brain"), throws. |
| UI | Kad's own design, replacing `app/pages.c` and `app/ui.c` screens over time. Keep the drawing layer. |
| Licence | The firmware as a whole is GPL-3.0-only. |

## The hardware, in one screen

- JieLi AC791N (WL82), pi32v2 CPU at about 360 MHz, **no external RAM**, 1 MiB single-bank flash, no debug header.
- 240x240 RGB565 screen; 27 keys (16 white F3..G5, 11 black); 14 buttons (FX SEL ENV LFO EDIT GLO HOME SAVE ARP SEQ
  PLAY REC OCT- OCT+); 7 encoders (SELECT, ALGORITHM, PRESETS, K1..K4); MASTER pot; USB-C; TRS MIDI in.
- 44.1 kHz audio. LOOM-8 renders in chunks of `SY_CHUNK` = 64 samples.
- A second core exists. Some units misread shared RAM and fm1-base then falls back to one core for good. **Design
  everything to work on one core.**
- RAM is the binding limit. LOOM-8 v0.14 ships with `pool_spare = 2048` in `app/fm1.toml` (fm1-base normally wants
  8 KB). Cut before you add.
- Newer units use UC25HQ80 flash (JEDEC 0xB36014). fm1-base handles it.

## Layout

```
base/                 fm1-base (MPL-2.0), git submodule; felucca-bsp inside as base/bsp. Do not edit here.
app/fm1.toml          build settings: name, id_prefix, USB names, optional parts, generated headers
app/app.h, app.c      glue to fm1-base; PROJ_FORMAT lives in app.h
app/engine.c          audio engine: sequencer tick, voices, sends, master; command queue main loop -> ISR
app/dsp/synth.c/.h    the voice pool and synth engines; TP_* parameter indices; EG_* engine enum
app/dsp/drumline.c    the drum kit (depends on synth.h's parameter layout)
app/dsp/fx.c/.h       send FX and master (self-contained: fx.h, fastmath.h, psxrev.h)
app/seq/pattern.h/.c  data model: tracks, patterns, steps, lock lanes, scenes, song
app/seq/seq.c/.h      the sequencer runtime
app/pages.c, ui.c     screens and input (about 10,800 lines; Kad's redesign replaces these over time)
app/project.c         save/load, format migration
tests/                run_tests.sh, tests/host/*_test.sh, tests/scenarios/*.xy
tools/                generators (fonts, wavetables, presets, samples), size pass, publishing
```

## Commands

```sh
# one-time (see BUILDING.md): Docker or podman on macOS; Python 3 with Pillow built with libraqm
base/tools/get_toolchain.sh                                   # JieLi clang 4.0.1 -> ~/.jieli/toolchain
git clone https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK.git -b AC79NN_SDK_V1.2.1_2023-12-13 --depth 1 ~/fw-AC79_AIoT_SDK

./build.sh                      # device package: app/build/cold.fwsc (+ cold-res.bin)
./build.sh --host               # host simulator: app/build/host/cold_host  (no JieLi toolchain needed)
./build.sh --web                # browser emulator: app/build/emu/  (needs Emscripten + Node)
tests/run_tests.sh              # the whole suite; run before every commit
tests/host/dsp_hash.sh save     # before a change that must not move a sample; the suite then compares
app/build/host/cold_host SCRIPT OUTDIR   # run one scripted scenario (commands: base/host/plat_host.c, app/host.c)

# hardware: only when Kad asks
python3 base/tools/fm1_install.py --info
python3 base/tools/fm1_install.py app/build/cold.fwsc
python3 base/tools/fm1_backup.py backup FILE --magic <MAGIC>
python3 base/tools/fm1_remote.py          # drive the FM-1 over USB, read screen, crash record, log
```

`JIELI_TOOLCHAIN` and `AC79_SDK` override the default paths. Long test runs: run in the background, log to a file.

## Rules

**Every commit**
- `./build.sh --host` and `tests/run_tests.sh` pass. One logical change per commit. If a change must move audio,
  say which fingerprints changed and why, then re-save them.
- After any change to RAM or flash use, run the device build and paste its image / RAM / pool figures into
  `docs/BUDGET.md` (one dated row per change). Never let pool spare go under 2 KB; the target is 8 KB.

**Code the build enforces (do not fight it)**
- No `double` anywhere on the device side (a soft-double routine fails the build). Floats only, no libm: use
  `fastmath.h`.
- No register access outside felucca-bsp. No flash calls from RAM code. No 32x32->64-bit multiply inlined next to
  16-bit loads (a CPU bug: `base/bsp/docs/CPU.md`); keep 64-bit products out of line, per block not per sample.

**Audio interrupt**
- `engine_render()` runs in the audio ISR. Never block, allocate or wait on the main loop there.
- The main loop reaches the ISR only through the engine's command queue (`engine_set`, `engine_key`, ...). Engine
  state is touched by one context only.
- Data the main loop hands the ISR in bulk (a whole patch) is published with a copy plus a generation counter, read
  at the start of a block. Follow that pattern (FMini-1's `fmini_set_patch` / `fmini_pgen` does the same).

**Saved data**
- `EG_*` engine numbers, `TP_*` parameter indices and other saved enums are stored in projects. **Append, never
  reorder or renumber.** Cut engines keep their enum slot (mark it retired) unless the project format is bumped
  with a migration.
- Any change to what a project stores bumps `PROJ_FORMAT` (app/app.h) and adds a migration in `app/project.c`.
  There are no users yet, so it is fine to drop old formats: say so in the commit.

**Upstream code**
- Do not edit `base/`. If fm1-base needs a change, write it up for Kad as an upstream issue or PR.
- Keep LOOM-8's history. Imported FMini-1 files keep their headers and authorship.

## Licensing (check before importing anything)

- The shipped firmware is GPL-3.0-only. Each file keeps a true `SPDX-License-Identifier` header. LOOM-8's own
  files stay MPL-2.0; FMini-1's files are GPL-3.0-only (Copyright Guilherme Ruppert, retained Felucca portions
  Leo Kuroshita / Hugelton Instruments).
- Every import gets a row in `LICENSING.md` (what, origin with commit, licence, where) and a line in the on-device
  credits and site credits (`app/fm1.toml` `credits`).
- Allowed sources: MIT, BSD, ISC, zlib, Apache-2.0, MPL-2.0, CC0, GPL-3.0 and GPL-2.0-or-later code; SIL OFL fonts.
  Not allowed: code with no licence, GPL-2.0-only, anything "non-commercial".
- No third-party patch or sample data without a licence that allows redistribution. FMini-1's 200 patches are the
  author's own and are fine.
- No trademarks in product, engine or preset names ("Minimoog", "OP-XY", "Elektron", "Hapax").

## Gotchas

- LOOM-8's drums and sequencer include `dsp/synth.h`; changing `TP_*` or `NTRK` ripples into both, the project
  format, MIDI channel mapping and the 10-channel USB audio (`usb_audio_channels` in fm1.toml). Hide before you
  remove: see PLAN.md phase 1.
- `EG_ACID` (the 303) is the template for a self-contained engine with its own filter and envelopes. `grep -n
  EG_ACID app/dsp/synth.c` lists every integration point (about 25).
- FMini-1's DEVELOPING.md describes v1.7.0; the port target is v2.5.0. Diff `firmware/src/eng_fmini*` between the
  tags before porting.
- FMini's ladder needs Q8 states (Q0 never self-oscillates), a noise nudge at note-on when emphasis >= 96, and a
  rounded multiply (`fmini_mulr`). Do not "simplify" these.
- Do not add host-side modulation behind an engine's back. Felucca's velocity "accent" opened FMini's filter by up
  to 1.7 octaves and was a bug.
- Host and device use different compilers. Host tests prove behaviour, not timing. CPU numbers come only from the
  device profiler (`plat_cpu_pct`, `plat_xruns`, LOOM-8's bench in `app/dsp/bench.c` with `XY_BENCH=1`).
- `tests/run_tests.sh` can hide warnings; the device build shows the ones that matter.

## Useful references

- fm1-base: `base/README.md`, `base/DESIGN.md`, `base/include/plat.h`, `base/skill/felucca-to-fm1-base/PORTING.md`
  (maps every Felucca file to its fm1-base replacement: useful for FMini's dependencies).
- FMini-1 (clone next to this repo at `../fmini-1`, tag v2.5.0): `DEVELOPING.md` sections 3 to 6 and 12,
  `firmware/src/eng_fmini.c`, `tools/fmini_filter.py`, `tools/fmini_bank.py`, `tools/gen_fmini_presets.py`.
- Recovery and the scene: github.com/cicloid/awesome-fm-1, github.com/Quixotic7/MvaveFM1Unbricker.
