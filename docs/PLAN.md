# PLAN.md

The build plan, in order. Work top to bottom, one checkbox per commit or small group of commits. Tick items as they
land and add a dated line under **Log** when a phase ends. If reality disagrees with the plan, stop and tell Kad
before changing it.

Status key: `[ ]` to do, `[x]` done, `[~]` in progress, `[-]` dropped (say why in the log).

---

## Phase 0: Setup and green baseline

Goal: an unmodified LOOM-8 that builds three ways, passes its tests, and carries this project's identity.

- [ ] Fork `charlesvestal/fm1-loom` at tag `v0.14` into Kad's GitHub. Clone with `--recursive`.
- [ ] Clone `ruppert/fmini-1` at `v2.5.0` next to it (`../fmini-1`), read-only reference.
- [ ] Toolchain and SDK installed (CLAUDE.md, Commands). Device build, `--host` and `--web` all succeed untouched.
- [ ] `tests/run_tests.sh` passes untouched. Save the log as `docs/baseline-tests.log` (gitignored if large).
- [ ] Record the untouched device build's image / RAM / pool figures as the first row of `docs/BUDGET.md`.
- [ ] Identity: the name is COLD (`name = "cold"` for build files). In `app/fm1.toml` set `name`, `usb_manufacturer`, `usb_product`,
      `tagline`, `repo`, `author`, `description`, `install`, `site`, `backup_ext`, and `APP_NAME` in `app/app.h`.
- [ ] `id_prefix`: every single digit after `FM-1_` is taken (LOOM-8 is `FM-1_10`, fm1-base's template `FM-1_11`,
      the Felucca family `FM-1_9xx`). Propose two unused two-digit prefixes to Kad after checking altstrument.org's
      FM-1 directory and fm1firmwarehub; Kad picks.
- [ ] Backup magic (`--magic`) and `APP_BACKUP_MAGIC`: new four-letter value so backups never cross with LOOM-8's.
- [ ] README rewritten as this project's (credit LOOM-8, fm1-base, FMini-1, Felucca at the top).
- [ ] Commit, tests green.

Done when: the renamed firmware builds three ways, tests pass, and the browser emulator shows the new name.

## Phase 1: Subtract

Goal: LOOM-8 cut to the decided scope with tests green after every cut and RAM recorded.

Order matters: hide in the UI first, remove code second, shrink data structures last.

**1a. Features the sequencer won't have**
- [ ] Weave: remove from UI, then engine, then data (`AUX_WEAVE`, `wvpat_t`). Project format bump.
- [ ] Conductor / brain (`AUX_BRAIN`, `TP_BRAIN`, `BM_*`): same order. Keep `TP_BRAIN` slot reserved.
- [ ] Throws / punch FX (`AUX_PUNCH`, `engine_punch`): ask Kad first; Elektron-style performance FX may be wanted.
- [ ] Players (arp, strum, hold, random): ask Kad which stay.

**1b. Engines**
- [ ] Ask Kad which 1 or 2 LOOM-8 synth engines stay (knit, rayon, jacquard, bellows, cane, voile, acid). Default if
      no answer: knit.
- [ ] For each removed engine: drop from the UI's engine list, drop its DSP and presets, keep its `EG_*` slot
      reserved. Remove its tables and samples from `gen` in fm1.toml if nothing else uses them.
- [ ] Keep the sample player. Ask Kad about the break slicer.
- [ ] If acid and the 909 models both go, note in LICENSING.md that LOOM-8's GPL parts left (the firmware stays
      GPL because of FMini).

**1c. Tracks 8 to 4**
- [ ] First hide tracks 5 to 8 in the UI only. Tests green.
- [ ] Then measure what `NTRK = 4` would save (pattern store, lanes, voices, USB audio channels) and show Kad the
      number before changing it.
- [ ] If yes: `NTRK` to 4, `usb_audio_channels` to 6 (master + 4 tracks), MIDI channel map, project format bump.

**1d. Optional platform parts** (only if RAM is still short; ask Kad, they are user-visible)
- [ ] `bt_audio` (Bluetooth audio out), chorus / lofi / phaser FX types. Bluetooth MIDI and screen mirroring stay.

- [ ] After each item: device build figures into `docs/BUDGET.md`.

Done when: 4 tracks, chosen engines only, no weave/brain, tests green, pool spare at least 8 KB.

## Phase 2: Port FMini-1's engine

Goal: FMini plays on a synth track, mono or 4-voice poly, with its 200 patches, host-tested.

- [ ] Diff FMini-1 `v1.7.0..v2.5.0` for `firmware/src/eng_fmini*`, `tools/fmini_*`, `tools/gen_fmini_presets.py`.
      Write a short note in `docs/fmini-port.md`: what changed since DEVELOPING.md was written.
- [ ] Copy into `app/dsp/fmini/`: `eng_fmini.c` (DSP parts only), `eng_fmini_filter.h`, `eng_fmini_presets.h`,
      and the generator scripts into `tools/fmini/`. Keep their headers. LICENSING.md row, credits line.
- [ ] Shim the Felucca dependencies it uses: `track_t`, `voice_t` (`s[8]` state, `keep` mask), `vmod_t`,
      `voice_amp`, `softclip`, `mulq15`, `mulq16`, `fmini_mulr`, `CTL`. Keep the engine integer (Q15 signals, Q8
      ladder states) inside; convert to float at the voice boundary. Check the float scale LOOM-8's engines render
      at (read `knit_loop` and `bass303_render` output) and match it.
- [ ] Add `EG_FMINI` **at the end** of the `EG_*` enum. Integrate it as a whole engine, following each `EG_ACID`
      point in `app/dsp/synth.c`: its own oscillators, filter and contours; LOOM-8's sends, mix and LFO around it.
      Voice state in a fixed pool (like `acid_mem[NACID]`), `NFMINI = 4`.
- [ ] Map parameters to `TP_*` so they are lockable: cutoff to `TP_CUT`, emphasis to `TP_RES`, contour amount to
      `TP_FENV`, filter contour to `TP_FA`/`TP_FD`/`TP_FS` (release = decay, as the original), loudness contour to
      `TP_A..TP_R`, LFO depth to the LFO page. The oscillator, mixer and switch bytes of the 32-byte patch go to
      `TP_E1..E4` / `TP_X1..X4` where they fit; propose the mapping to Kad before coding the rest.
- [ ] Patch hand-off main loop to ISR with copy + generation counter, as `fmini_set_patch` does.
- [ ] Voice modes: MONO (lowest note, retrigger, glide) and POLY capped at 4. Respect LOOM-8's `TP_VMODE`.
- [ ] FMini selectable from the engine list on any of the 3 synth tracks, like LOOM-8's own engines.
- [ ] Presets: the 200 factory patches selectable with PRESETS on an FMini track. Keep `fmini_levels.json`
      loudness.
- [ ] Tests: port `tests/fmini_filter_test.c` checks to the host (self-oscillation pitch, 1 V/oct over 4 octaves,
      emphasis table, boundedness), a render fingerprint per patch, velocity 20..127 does not move the cutoff.
- [ ] A/B: render 10 patches on the host to WAV; Kad compares against FMini-1's browser emulator
      (ruppert.github.io/fmini-1/emu/).
- [ ] Static cost: add the FMini render functions to LOOM-8's instruction count check (`tests/host/instr_count.h`,
      `eng_bench`).

Done when: an FMini track plays all 200 patches mono and in 4-voice poly on the host and in the browser emulator,
p-locks move its cutoff and emphasis, tests green, budget recorded.

## Phase 3: First hardware check (Kad flashes)

- [ ] Prepare a release-style package and the exact install and restore commands. Stop and hand to Kad.
- [ ] With Kad: all 4 tracks playing (FMini poly 4 + the other synth + drums), delay and reverb on. Read CPU % and
      xruns with `fm1_remote.py` / the profiler. Record in `docs/BUDGET.md`.
- [ ] Set the FMini voice cap from the measurement (aim for at most 75% CPU worst case on one core).
- [ ] Note anything that sounds wrong; fix on the host; repeat.

Done when: the box plays a full pattern on hardware without dropouts on one core.

## Phase 4: Sequencer discovery (with Kad)

Goal: numbers to design from, before rewriting screens.

- [ ] Write host scenario scripts (`tests/scenarios/ux_*.xy`) for the five jobs, on the current (LOOM-8) UI:
      1. a beat (kick, snare, hats, one fill); 2. a 16-step bassline with two slides; 3. two chords, one held 8 steps;
      4. a variation of pattern 1 and a switch to it on the bar; 5. change one sound while the pattern plays.
      Count presses and page changes, save contact sheets of screenshots. Write `docs/ux-baseline.md`.
- [ ] Read LOOM-8's `docs/ux-review.md` findings with Kad; mark which the new design must fix.
- [ ] Kad sketches screens. For each sketch, script the same five jobs against a mock (or the first prototype) and
      compare counts.
- [ ] Hapax ideas to evaluate with Kad (do not build unasked): per-track length; per-track speed as ratios (1/2,
      2/3, 3/4, 1, 3/2, 2); project scale with in-scale keys lit; track transpose; one snapshot per track; at most
      two per-track generators (scaler, Euclidean or random); live record quantised to the step.
- [ ] Keep LOOM-8's pattern data model unless a design truly needs a change. Any data change = format bump + Kad
      sign-off.

## Phase 5: New UI

- [ ] Build Kad's screens page by page, replacing the matching parts of `pages.c` / `ui.c`. Old and new may coexist
      behind a build flag until the new one covers the five jobs.
- [ ] Every screen in the layout audit; screenshots regenerated (`tools/gen_screens.sh`).
- [ ] P-lock display and clearing per Kad's design. Trig conditions reachable from the step page.
- [ ] Rerun the five-job scripts; numbers must beat `docs/ux-baseline.md`.
- [ ] Manual rewritten (`docs/MANUAL.md`).

## Phase 6: Release

- [ ] `.github/workflows/release.yml` pointing at this repo; the shared builder `zvenson/fm1-build@v1`.
- [ ] LICENSING.md and CREDITS complete; on-device credits screen; site credits.
- [ ] Release notes draft; Kad tags; site published with the web installer and emulator.

---

## Open questions for Kad

- `id_prefix` (the name is COLD).
- Which 1 or 2 LOOM-8 synth engines stay.
- Throws / performance FX: keep, change or cut.
- Which players (arp, strum, hold, random) stay.

## Log

- 2026-11-09: plan written. Decisions: fork LOOM-8 v0.14; FMini as main synth (mono or 4-voice poly); any
  synth track can pick any engine; Bluetooth MIDI and screen mirroring stay; Kad designs sequencer and UI.
