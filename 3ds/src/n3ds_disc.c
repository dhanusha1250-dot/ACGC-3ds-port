/* n3ds_disc.c - Disc image reader (SCAFFOLD).
 *
 * PC equivalent: pc/src/pc_disc.c + pc/src/pc_dvd.c.
 *
 * pc_disc.c is platform-agnostic stdio - it uses fopen/fread/fseek to read
 * CISO/ISO/GCM disc images. devkitARM's newlib exposes fopen on:
 *
 *   sdmc:/        SD card root
 *   romfs:/       application's read-only romfs (in the .3dsx / .cia)
 *
 * Plan:
 *   - Disc image lives on SD card at sdmc:/3ds/AnimalCrossing/rom/<file>
 *   - Save files: sdmc:/3ds/AnimalCrossing/save/<file>.gci
 *   - Texture pack: sdmc:/3ds/AnimalCrossing/texture_pack/ (likely OOM on
 *     Old 3DS; gate behind New-3DS-only or skip entirely)
 *
 * Implementation: in most cases pc_disc.c can be reused with TARGET_3DS
 * substituted for TARGET_PC and the rom path scanned from the SD location
 * above. This file currently just documents that approach.
 */

#include "n3ds_platform.h"

/* TODO: thin wrapper that selects the SD path and delegates into pc_disc.c
 * once we link the PC layer into the 3DS build, OR a clean fork that drops
 * the Windows-specific dirent shim. */
