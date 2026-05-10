/* n3ds_pad.c - 3DS HID -> GameCube PAD (SCAFFOLD).
 *
 * PC equivalent: pc/src/pc_pad.c.
 *
 * The decomp game calls PADRead() expecting a PADStatus[4] with GC buttons,
 * 8-bit signed analog/c-stick, and 8-bit triggers. On 3DS:
 *
 *   - hidScanInput() once per frame
 *   - hidKeysHeld() / hidKeysDown() for the digital buttons
 *   - hidCircleRead(&pos) for the left stick (range roughly +-150)
 *   - hidCstickRead(&pos) for the C-stick (New 3DS only - guard via
 *     osGetSystemVersionData; on Old 3DS leave c-stick at 0 or map to
 *     touch / shoulder + circle pad as a substitute)
 *   - Old 3DS has no analog triggers; map ZL/ZR / L/R as full-on digital
 *     triggers (0 or 255). Z trigger needs another binding (e.g. touch
 *     bottom-screen region).
 *
 * Button mapping (proposed default, tweakable later via a config file):
 *   A      -> A
 *   B      -> B
 *   X      -> X
 *   Y      -> Y
 *   START  -> START
 *   L      -> L
 *   R      -> R
 *   ZL     -> Z
 *   ZR     -> (unused / spare)
 *   DPad   -> DPad
 *   Circle -> Main stick
 *   CStick -> C-stick (N3DS only)
 *
 * Rumble: 3DS itself has no rumble; the Circle Pad Pro accessory does, but
 * dropping rumble entirely is the realistic choice.
 */

#include "n3ds_platform.h"

/* TODO: PADInit, PADRead matching include/dolphin/pad.h. */
