# Porting Notes: Animal Crossing PC Port -> Nintendo 3DS

This document maps the existing PC port (`pc/`) onto Nintendo 3DS hardware
via devkitARM, libctru, and citro3d. It is a planning document — the actual
3DS port is at scaffold stage (`3ds/`).

## TL;DR

- ARM11 is 32-bit, so the project's hard requirement of `sizeof(void*) == 4`
  (JSystem casts pointers to `u32`) is satisfied natively.
- Both PC and 3DS are little-endian, so save-byteswap code is unchanged.
- The graphics layer is the largest piece of work. PICA200 is not OpenGL;
  shaders are PICA assembly (`.v.pica`) compiled with picasso, and the
  fragment stage is a fixed-function TEV combiner with 6 stages.
- Memory is tight: the PC port reserves 24 MB main + 16 MB ARAM = 40 MB.
  Old 3DS apps get ~64 MB, with citro3d eating some of that. Realistic
  target is **New 3DS**.
- Performance is the unresolved risk. GC was 485 MHz PowerPC; Old 3DS is
  268 MHz ARM11. Even a "perfect" port will not run full-speed on Old 3DS.

## Subsystem mapping

| PC subsystem                | File                         | 3DS replacement                                   |
|-----------------------------|------------------------------|--------------------------------------------------|
| Window + GL context         | `pc_main.c` (SDL2/GL)        | `gfxInitDefault` + `C3D_Init` + render targets   |
| Event loop                  | `pc_main.c` (`SDL_PollEvent`)| `aptMainLoop` + `hidScanInput` per frame         |
| GX -> graphics API          | `pc_gx.c`, `pc_gx_tev.c`, `pc_gx_texture.c` | citro3d (PICA200) + picasso shaders |
| Audio                       | `pc_audio.c` (SDL2)          | ndsp, channel 0, NDSP_FORMAT_STEREO_PCM16        |
| Threading                   | SDL_Thread / SDL_atomic_     | `threadCreate` / `LightLock` / `LightEvent`      |
| Controller input            | `pc_pad.c` (SDL gamepad)     | hid: `hidKeysHeld`, `hidCircleRead`, `hidCstickRead` |
| Disc image read             | `pc_disc.c`, `pc_dvd.c`      | stdio on `sdmc:/3ds/AnimalCrossing/rom/`         |
| Memory card / save          | `pc_card.c`, `pc_m_card.c`   | stdio on `sdmc:/3ds/AnimalCrossing/save/` (GCI)  |
| ARAM emulation              | `pc_aram.c`                  | unchanged (plain `malloc`)                       |
| Asset extraction            | `pc_assets.c` (30k LOC)      | unchanged                                        |
| Endianness                  | `pc_save_bswap.c`            | unchanged (both targets are LE)                  |
| Crash protection            | `pc_main.c` VEH / sigaction  | `svcSetExceptionHandler` (optional, post-MVP)   |
| Keyboard / typing UX        | `pc_typing.c`, `pc_text_draw.c` | drop `KEYBOARD_TYPING` define; use 3DS SWKBD via `swkbdInit` |
| Pause menu / settings menu  | `pc_pause_menu.c`, `pc_settings_menu.c` | re-author with citro2d / SF2D-style |
| Texture pack                | `pc_texture_pack.c`          | New 3DS only (Old 3DS: out of memory)            |
| Model viewer (debug)        | `pc_model_viewer.c`          | optional, gate behind compile-time flag          |
| NES emulator (in-game NES)  | `pc_nes_fixnes.c`            | unchanged C code; verify ARM perf                |

## Graphics: GX -> PICA200

This is the largest piece of net-new work. PICA200 is closer to a
fixed-function pipeline than to GL3.3:

- **Vertex shader**: programmable, but very limited — 96 instruction slots,
  16 vec4 input attrs, 16 outputs, no flow control beyond `call`/`jmp`/
  `cmp` + `breakc`. Most uniform setup is one mat4 + ambient color.
- **Fragment stage**: not programmable. 6 TEV combiner stages (cf. GX's 16).
  Configure via `C3D_TexEnv`, output a final color. Materials that need
  more than 6 stages must split into multiple draws.
- **Texture formats**: RGBA8, RGB8, RGBA5551, RGB565, RGBA4, LA8, HILO8, L8,
  A8, LA4, L4, A4, ETC1, ETC1A4. CMPR/CI8/IA8 from GX have to be transcoded
  at load. RGBA8 must be tiled into 8x8 blocks (use libctru's
  `GX_DisplayTransfer` or `GSPGPU_FlushDataCache` + tiling helpers).
- **EFB copy**: GX's `GXCopyTex` becomes "render to an off-screen
  `C3D_RenderTarget` and bind it as a texture in the next pass".
- **Screen layout**: Top screen is 400x240 (or 800x240 stereo), bottom is
  320x240. Pick a strategy:
  - **A: Game on top, UI on bottom.** Most natural fit. Bottom can show
    inventory / map / pause menu and accept stylus input.
  - **B: Game on both screens.** Mirror the GC's 640x480 framebuffer split
    into two strips. Looks wrong for a game designed for a single screen.
  - Recommend (A).
- **Resolution**: rendering at native GC 640x480 then downscaling on present
  is expensive. Render at top-screen native (400x240) and adapt UI layouts
  is cheaper, but breaks asset alignment assumptions everywhere in the
  decomp. **MVP: keep 640x480 internal, blit-downsample to 400x240.**

Concrete graphics work order:

1. Author `n3ds_default.v.pica` (mat4 mvp + position/color/uv passthrough)
   and a minimal TEV setup.
2. Implement `GXSetViewport`, `GXSetProjection`, `GXLoadPosMtxImm`,
   `GXSetVtxAttrFmt`, `GXBegin/End` (the immediate-mode draw entry point
   the decomp uses) as the smallest viable surface for getting the title
   screen on screen.
3. Texture: `GXInitTexObj` -> `C3D_Tex` + `C3D_TexInit`, with a transcode
   path per `GX_TF_*` format.
4. TEV: `GXSetTevColorIn / GXSetTevAlphaIn / GXSetTevColorOp / GXSetTevAlphaOp`
   -> `C3D_TexEnvSrc` / `C3D_TexEnvFunc` / `C3D_TexEnvScale`. Bail out
   (or split) past 6 stages.
5. EFB copy: `GXCopyTex` -> render to off-screen target + `C3D_TexBindCube`-
   style binding.

## Audio

ndsp model is simpler than SDL2's:

```
ndspInit();
ndspChnReset(0);
ndspChnSetFormat(0, NDSP_FORMAT_STEREO_PCM16);
ndspChnSetRate(0, 32000.0f);
ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
/* allocate two ndspWaveBuf in linearAlloc'd memory */
/* per frame: if buffer marked done, refill + ndspChnWaveBufAdd */
```

The PC port's producer thread structure ports almost 1:1 — replace
`SDL_CreateThread` with `threadCreate(..., prio=0x18, processor=1, ...)`
after calling `APT_SetAppCpuTimeLimit(30)` so core 1 is available for app
threads.

## Input

GameCube PAD model -> 3DS HID:

| GC PADStatus field   | 3DS source                               |
|----------------------|------------------------------------------|
| button.a/b/x/y/start | `KEY_A` / `KEY_B` / `KEY_X` / `KEY_Y` / `KEY_START` |
| dpad                 | `KEY_DUP/DOWN/LEFT/RIGHT`                |
| stickX, stickY       | `hidCircleRead()` scaled to s8           |
| substickX, substickY | `hidCstickRead()` (N3DS only)            |
| triggerL, triggerR   | `KEY_L` / `KEY_R` -> 0 or 255            |
| Z trigger            | `KEY_ZL` (N3DS) or touch hot region (O3DS) |
| rumble               | drop (3DS has no rumble)                 |

## Memory budget

Old 3DS application title: ~64 MB usable RAM. Breakdown:

| Region                | Size      | Notes                          |
|-----------------------|-----------|--------------------------------|
| Game main memory      | 24 MB     | `N3DS_MAIN_MEMORY_SIZE`        |
| ARAM emulation        | 16 MB     | `N3DS_ARAM_SIZE` (heap-backed) |
| citro3d + render tgts | ~6 MB     | top RT (RGBA8 + depth) + off-screen |
| Audio buffers         | <1 MB     | 2x ndspWaveBuf                 |
| libctru + newlib + stack | ~3 MB  |                                |
| **Subtotal**          | **~50 MB**| Leaves ~14 MB headroom         |

The 14 MB headroom is consumed by asset working set, texture residency, and
fragmentation. Realistically: cut ARAM to 8 MB and gate texture pack to
N3DS-only.

**New 3DS (128 MB)** has all the headroom needed. Target this first.

## Required system calls / titles

- `APT_SetAppCpuTimeLimit(30)` — release core 1 for the audio thread.
- `osSetSpeedupEnable(true)` — bump CPU/L2 on New 3DS (clocks up to 804 MHz).
- Extended memory mode in the RSF (for .cia) or default heap (3dsx): the
  .3dsx loader gives ~24 MB heap by default; the RSF for a CIA can request
  the full 96 MB / 124 MB extended modes.

## Milestones

1. **M0 - Scaffold** (this commit): Makefile, hello-world boot, stub files.
2. **M1 - GX surface**: implement the ~30 GX entry points the title screen
   touches; get a single textured quad rendering.
3. **M2 - Disc + assets**: read disc image from SD, run asset extraction.
4. **M3 - Pad + audio**: PADRead returns sensible values, ndsp plays SFX.
5. **M4 - Title screen**: enough of the engine wired to render the title.
6. **M5 - In-game**: village renders, player moves. Performance audit.
7. **M6 - Save + UI**: GCI save IO, redesigned pause menu on bottom screen.

Each milestone is multi-day-to-weeks of work in its own right.

## Out of scope / explicit non-goals

- HD texture pack on Old 3DS (won't fit).
- Keyboard typing (`KEYBOARD_TYPING` define) — use 3DS SWKBD instead.
- 3D (stereoscopic) rendering — not impossible but adds a 2x perf cost.
- Online features — not part of the PC port either.
