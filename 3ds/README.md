# 3DS Port (scaffold)

This directory mirrors `pc/` but targets the Nintendo 3DS via devkitARM,
libctru, and citro3d.

**Status: scaffold only.** What's here today:

- `Makefile` — devkitARM build that produces `AnimalCrossing.3dsx`
- `src/main.c` — boots libctru + citro3d, clears the top screen, opens a
  console on the bottom screen, polls input, exits cleanly on START
- `src/n3ds_*.c` — subsystem stub files matching the `pc/src/pc_*.c` layout.
  Each one documents the 3DS equivalents and what needs to be implemented.
- `include/n3ds_platform.h` — mirrors `pc/include/pc_platform.h`

The actual game code is **not** wired in yet. None of the decomp `src/` or
`pc/src/pc_*.c` modules are referenced by this build. See
`../docs/3DS_PORTING.md` for the analysis and porting plan.

## Build

Requires devkitPro + devkitARM with the 3ds-dev meta-package:

```
sudo dkp-pacman -S 3ds-dev
source /etc/profile.d/devkit-env.sh
cd 3ds
make
```

Output: `AnimalCrossing.3dsx`. Copy to your SD card under `/3ds/` and launch
via the Homebrew Launcher, or run under Citra/Lime3DS:

```
citra-qt AnimalCrossing.3dsx
```

## Caveats

- The build was authored from spec; it has **not** been verified to compile
  in this sandbox (no devkitARM available). The Makefile is the standard
  3ds-examples template adapted to this project layout — expect to fix one
  or two paths the first time you build.
- Performance is the unresolved risk. GC was a 485 MHz PowerPC; Old 3DS is
  a 268 MHz ARM11. The original game will almost certainly not run at full
  speed even with a complete port. New 3DS (804 MHz, 4 cores, 128 MB) is the
  realistic target.
- Memory budget is tight: the PC port reserves 24 MB main RAM + 16 MB ARAM.
  Old 3DS apps get ~64 MB; the budget fits but leaves little room for
  citro3d resources. Texture pack support should be New-3DS-only.
