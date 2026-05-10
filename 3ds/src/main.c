/* main.c - 3DS entry point.
 *
 * Scaffold hello-world: boots libctru, initializes citro3d, clears the top and
 * bottom screens to distinct colors, polls input, and exits cleanly on START.
 *
 * When the real port is wired in, this file's main() will end up doing roughly
 * what pc/src/pc_main.c does: parse argv-equivalent (3dsx --param or config
 * file), call n3ds_platform_init(), initialize disc/asset/audio subsystems,
 * call ac_entry() to register the game's HotStartEntry, then call boot_main()
 * to enter the game loop.
 */
#include <stdio.h>
#include <string.h>

#include <3ds.h>
#include <citro3d.h>

#include "n3ds_platform.h"

/* --- Globals (mirrors pc_main.c) --- */
int g_n3ds_running = 1;
int g_n3ds_verbose = 0;

/* citro3d render targets - one per physical screen.
 * Top screen is 400x240 (or 800x240 in stereo); bottom is 320x240. */
static C3D_RenderTarget* s_target_top = NULL;
static C3D_RenderTarget* s_target_bot = NULL;

/* GPU command buffer size. 0x40000 (256KB) matches the libctru default and is
 * roughly equivalent to pc_platform.h's PC_FIFO_SIZE. */
#define N3DS_GPU_CMDBUF_SIZE 0x40000

/* --- Console for debug log on bottom screen ---
 * We use libctru's consoleInit on the bottom screen during scaffolding so we
 * can see printf() output on real hardware. Once the real game wiring lands
 * the bottom screen will become a game UI target instead. */
static int s_console_active = 0;

void n3ds_platform_init(void) {
    gfxInitDefault();

    /* Bottom screen as text console for scaffolding. */
    consoleInit(GFX_BOTTOM, NULL);
    s_console_active = 1;

    /* Initialize citro3d on the top screen only for now. */
    if (!C3D_Init(N3DS_GPU_CMDBUF_SIZE)) {
        printf("[3DS] C3D_Init failed\n");
        /* Stay alive long enough for the user to see the message. */
        for (int i = 0; i < 240 && aptMainLoop(); i++) {
            gspWaitForVBlank();
            gfxSwapBuffers();
        }
        gfxExit();
        return;
    }

    s_target_top = C3D_RenderTargetCreate(N3DS_TOP_HEIGHT, N3DS_TOP_WIDTH,
                                          GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(s_target_top, GFX_TOP, GFX_LEFT,
                              GX_TRANSFER_FLIP_VERT(0)
                              | GX_TRANSFER_OUT_TILED(0)
                              | GX_TRANSFER_RAW_COPY(0)
                              | GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGBA8)
                              | GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB8)
                              | GX_TRANSFER_SCALING(GX_TRANSFER_SCALE_NO));

    printf("Animal Crossing 3DS port - scaffold\n");
    printf("\n");
    printf("This is a placeholder boot. None of the\n");
    printf("game code is wired in yet.\n");
    printf("\n");
    printf("Press START to exit.\n");
}

void n3ds_platform_shutdown(void) {
    if (s_target_top) {
        C3D_RenderTargetDelete(s_target_top);
        s_target_top = NULL;
    }
    if (s_target_bot) {
        C3D_RenderTargetDelete(s_target_bot);
        s_target_bot = NULL;
    }
    C3D_Fini();
    gfxExit();
    s_console_active = 0;
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

void n3ds_platform_swap_buffers(void) {
    /* Clear + present top screen. Bottom screen is owned by the libctru console
     * during scaffolding so we don't touch its framebuffers here. */
    if (s_target_top) {
        C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
        C3D_RenderTargetClear(s_target_top, C3D_CLEAR_ALL, 0x202060FF, 0);
        C3D_FrameDrawOn(s_target_top);
        C3D_FrameEnd(0);
    } else {
        gspWaitForVBlank();
        gfxSwapBuffers();
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    n3ds_platform_init();

    while (g_n3ds_running) {
        if (!n3ds_platform_frame()) break;
        n3ds_platform_swap_buffers();
    }

    n3ds_platform_shutdown();
    return 0;
}
