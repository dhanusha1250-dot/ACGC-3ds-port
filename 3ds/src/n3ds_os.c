/* n3ds_os.c - OS shims: time, threads, memory (SCAFFOLD).
 *
 * PC equivalent: pc/src/pc_os.c.
 *
 * The decomp uses Dolphin OS APIs (OSGetTime, OSCreateThread, OSGetTick,
 * OSReport, ...). Mappings:
 *
 *   OSGetTime / OSGetTick    -> svcGetSystemTick() (return cycles at 268MHz,
 *                              scale into GC_TIMER_CLOCK units to keep the
 *                              decomp's math working without changes)
 *   OSCreateThread           -> threadCreate (libctru)
 *   OSResumeThread           -> the thread is started immediately by
 *                              threadCreate; this becomes a no-op or a
 *                              LightEvent wakeup
 *   OSCalendarTime           -> time(NULL) + localtime_r work via newlib
 *   OSReport                 -> printf (goes to libctru console on bottom
 *                              screen during scaffold; redirect to file
 *                              in release)
 *
 * APT_SetAppCpuTimeLimit(30) is required at startup to let the audio
 * producer thread run on core 1. Without it, threadCreate on processor 1
 * silently spins on core 0 and audio will glitch.
 *
 * Memory: extended heap mode (via meta in the .3dsx / RSF for .cia) is
 * required to get more than ~32MB linear heap on Old 3DS. New 3DS has
 * substantially more. See docs/3DS_PORTING.md for the budget breakdown.
 */

#include "n3ds_platform.h"

/* TODO: OS API mapping table. */
