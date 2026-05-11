/* n3ds_main.c - 3DS entry point (mirrors pc/src/pc_main.c).
 *
 * Default build (WIRE_DECOMP=0) is the scaffold proof-of-life:
 *   - libctru gfxInitDefault + console on the bottom screen
 *   - n3ds_gx layer brings up citro3d + a PICA vertex shader
 *   - per-frame: clear top screen, draw a single colored test triangle
 *   - START exits cleanly
 *
 * The test triangle goes away as soon as real GX entry points start
 * landing in n3ds_gx.c -- it exists only to confirm the pipeline.
 *
 * `make WIRE_DECOMP=1` instead calls the decomp's renamed entry points
 * (ac_entry / boot_main). That build still won't link until the rest of
 * the subsystems are stubbed; see docs/3DS_PORTING.md.
 *
 * Filename note: this is n3ds_main.c (not main.c) because the devkitARM
 * template flattens object files to basenames and would otherwise collide
 * with the decomp's src/main.c.
 */
#include <stdio.h>
#include <string.h>

#include <3ds.h>
#include <citro3d.h>

#include "n3ds_platform.h"
#include "n3ds_gx_internal.h"

/* --- Decomp entry points (see pc/src/pc_main.c:259-260) ---
 * Per-file -Dmain=... renames in 3ds/Makefile turn the two decomp main()s
 * into these symbols. Only active under WIRE_DECOMP=1. */
#ifdef N3DS_WIRE_DECOMP
extern void ac_entry(void);
extern int  boot_main(int argc, const char** argv);
#endif

/* --- Globals (mirrors pc_main.c) --- */
int g_n3ds_running = 1;
int g_n3ds_verbose = 0;

void n3ds_platform_init(void) {
    gfxInitDefault();

    /* Bottom screen as text console for scaffolding. Once the real game
     * UI lands, the bottom screen becomes a citro3d render target instead. */
    consoleInit(GFX_BOTTOM, NULL);

    printf("Animal Crossing 3DS port - scaffold\n");
    printf("\n");
    printf("citro3d test draw: a colored triangle\n");
    printf("should appear on the top screen.\n");
    printf("\n");
    printf("Press START to exit.\n");
}

void n3ds_platform_shutdown(void) {
    gfxExit();
}

int n3ds_platform_frame(void) {
    if (!aptMainLoop()) {
        g_n3ds_running = 0;
        return 0;
    }
    hidScanInput();
    u32 kDown = hidKeysDown();
    if (kDown & KEY_START) {
        g_n3ds_running = 0;
        return 0;
    }
    return 1;
}

/* Wait until SELECT is pressed (or HOME via aptMainLoop). Used to keep the
 * console visible after the main loop exits so trace logs are readable. */
static void n3ds_wait_for_exit(void) {
    printf("\nPress SELECT to exit.\n");
    while (aptMainLoop()) {
        hidScanInput();
        if (hidKeysDown() & KEY_SELECT) break;
        gspWaitForVBlank();
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    n3ds_platform_init();

    int gx_ok = n3ds_gx_init();

    if (gx_ok) {
#ifdef N3DS_WIRE_DECOMP
        /* TODO: n3ds_disc_init(), n3ds_assets_init() before this once those land. */
        ac_entry();              /* registers HotStartEntry */
        boot_main(0, NULL);      /* runs HotStartEntry loop; returns when game exits */
#else
        /* Scaffold mode: draw a test triangle every frame to prove the GX
         * pipeline is alive. START exits the draw loop (not the app). */
        while (g_n3ds_running) {
            if (!n3ds_platform_frame()) break;
            n3ds_gx_begin_frame();
            n3ds_gx_draw_test_triangle();
            n3ds_gx_end_frame();
        }
        printf("\nDraw loop exited (START pressed).\n");
#endif
    } else {
        printf("\n[GX] init failed -- running console-only.\n");
    }

    n3ds_wait_for_exit();

    n3ds_gx_shutdown();
    n3ds_platform_shutdown();
    return gx_ok ? 0 : 1;
}
