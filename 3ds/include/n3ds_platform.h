/* n3ds_platform.h - 3DS platform layer, mirrors pc/include/pc_platform.h.
 *
 * Names use the n3ds_ prefix to avoid colliding with libctru's `_3ds_*` and to
 * make grep across the tree unambiguous. (The `n` is just a tag; this targets
 * both Old 3DS and New 3DS.)
 *
 * Scaffold status: globals + init/shutdown signatures only. None of the real
 * GX/audio/input/disc subsystems are implemented yet.
 */
#ifndef N3DS_PLATFORM_H
#define N3DS_PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

/* 32-bit pointers required (JSystem casts pointers to u32).
 * ARM11 is 32-bit so this is satisfied natively. */
#if defined(__3DS__) && (UINTPTR_MAX != 0xFFFFFFFFu)
#error "3DS build must produce 32-bit pointers"
#endif

#ifdef __3DS__
#include <3ds.h>
#include <citro3d.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* --- Screen geometry --- */
#define N3DS_TOP_WIDTH    400
#define N3DS_TOP_HEIGHT   240
#define N3DS_BOT_WIDTH    320
#define N3DS_BOT_HEIGHT   240

/* GC native render target. We still tell the game we're rendering at 640x480
 * and present a downsampled image, the same way the PC port does. */
#define N3DS_GC_WIDTH     640
#define N3DS_GC_HEIGHT    480

/* PC port keeps these constants. Mirror them so anything that links against
 * both layers sees consistent values. */
#define N3DS_MAIN_MEMORY_SIZE   (24 * 1024 * 1024)
#define N3DS_ARAM_SIZE          (16 * 1024 * 1024)
#define N3DS_FIFO_SIZE          (256 * 1024)
/* WARNING: 24MB + 16MB = 40MB working set. Old 3DS apps get ~64MB total.
 * This will need APT_SetAppCpuTimeLimit + extended memory mode, and probably
 * heavy trimming before it fits. See docs/3DS_PORTING.md. */

/* --- Global state --- */
extern int  g_n3ds_running;
extern int  g_n3ds_verbose;

/* --- Lifecycle --- */
void n3ds_platform_init(void);
void n3ds_platform_shutdown(void);

/* Returns 0 when the app should exit (e.g. HOME pressed and aptMainLoop says
 * stop). Otherwise returns 1. */
int  n3ds_platform_frame(void);

/* Present the current frame to both screens. */
void n3ds_platform_swap_buffers(void);

#ifdef __cplusplus
}
#endif

#endif /* N3DS_PLATFORM_H */
