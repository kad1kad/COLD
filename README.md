# COLD for the M-VAVE FM-1

COLD is a minimal four-track groovebox for the M-VAVE FM-1, by Kad. It is a work in progress: three synth tracks
and one drum track, with the synth engine from FMini-1 as the main voice and a sequencer interface designed from
scratch.

COLD stands on other people's work, and they come first:

- **[LOOM-8](https://github.com/charlesvestal/fm1-loom)** by Charles Vestal (v0.14). COLD is a fork of it, cut down.
  The sequencer, the parameter locks, the scenes, the song, the drum kit, the effects and the tooling all come from LOOM-8.
- **[fm1-base](https://github.com/charlesvestal/fm1-base) and [felucca-bsp](https://github.com/hugelton/felucca-bsp)**, the platform under it
  (the `base/` submodule), from [Felucca](https://github.com/hugelton/Felucca) by Leo Kuroshita (Hügelton Instruments).
- **[FMini-1](https://github.com/ruppert/fmini-1)** by Guilherme Ruppert (v2.5.0), whose synth engine and 200 factory
  patches are being ported in. Not in yet.

## Where it is today

So far COLD has five tracks (T1 kit, T2 to T5 synths), the engines rayon and jacquard, the sample player and MIDI out,
and LOOM-8's sequencer without weave. FMini-1's engine is not in yet. The conductor is hidden but still in the code. The plan is in [docs/PLAN.md](docs/PLAN.md). The manual (`docs/MANUAL.md`) and the screenshots
are still LOOM-8's and will be rewritten with the new interface.

Nothing here has been tested on a real FM-1 under the COLD name. Installing custom firmware is at your own risk.
Do not install it unless you know how to put M-VAVE's firmware back.

## Building and testing

COLD is an app (`app/`) on fm1-base, a git submodule (`base/`): clone with `--recursive`. See `BUILDING.md`.
`tests/run_tests.sh` runs everything on the host, silently (the audio goes to WAV files). `LICENSING.md` says
where the code comes from. COLD is GPL-3.0-only as a whole (`COPYING`); most of LOOM-8's files are
MPL-2.0 (`LICENSE`), each on its own.

## Credits

The platform: fm1-base and felucca-bsp (MPL-2.0), from Felucca by Leo Kuroshita (Hügelton Instruments), with his
permission (felucca-bsp#4). The reverb's PlayStation modes: PSXVerb (MIT). The x drums: after MACHINEDRUM by Omodaka9375 (MIT); sophie: Sophie by Matt Estela (MIT); clang: digisophie (MIT). The factory breaks: free loops by u/FCTRYBEATS,
with their permission; the plate reverb and the 909 kick, snare and clap: 9W9 by athousanddetails, under MPL-2.0 with his permission; the 909 kit models: 9W9's circuits by athousanddetails (GPL-3.0); the 909 sampler's speed-ups: Optimist (GPL-3.0); the 909 hat: ER-99's sample by Matthew Cieplak, through 9W9 (GPL-3.0, in the sound bank only); the 909 cymbals (hat, open hat, ride, crash): ER-99's samples by Matthew Cieplak, through 9W9, played by 9W9's sampler (GPL-3.0, in the resource region beside the firmware); the recorded instruments: Versilian Studios Chamber
Orchestra 2 Community Edition (CC0). The fonts: Barlow and Terminus (SIL OFL 1.1). The second core's lessons: Melodee (keremimo) and Jangada (zednaked). The USB MIDI host: Melodee's, by Adrien Tibi and Kerem Kilic (MPL-2.0, in felucca-bsp).
Bluetooth MIDI and Bluetooth audio: Optimist's own stack by Henri David (MPL-2.0, in fm1-base), with Android's SBC
encoder (Apache-2.0) and micro-ecc by Kenneth MacKay (BSD-2-Clause); the radio's start-up tables and calibration
after JieLi's AC79 SDK (Apache-2.0).
The idea of a browser emulator: Groove OS.

LOOM-8's way of working owes a great deal to teenage engineering's OP-XY; COLD is not affiliated with
teenage engineering.
