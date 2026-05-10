/* n3ds_gx.c - GX -> PICA200 translation layer (SCAFFOLD).
 *
 * PC equivalent: pc/src/pc_gx.c, pc_gx_tev.c, pc_gx_texture.c.
 *
 * The PC port reinterprets GameCube GX commands as OpenGL 3.3 calls. On 3DS
 * the destination is PICA200 via citro3d. The shape of the work is similar
 * but the differences matter:
 *
 *   - GLSL is not available. Vertex and fragment shaders are written in PICA
 *     assembly (.v.pica / .f.pica) and compiled with picasso. The fragment
 *     stage is a fixed-function combiner pipeline, not a programmable one.
 *
 *   - PICA200 has 6 TEV stages; GX has up to 16. Materials that chain more
 *     than 6 stages must be split into multiple draw passes, or simplified.
 *
 *   - Texture formats: PICA supports RGBA8, RGB8, RGBA5551, RGB565, RGBA4,
 *     LA8, HILO8, L8, A8, LA4, L4, A4, ETC1, ETC1A4. Map GX_TF_* to these.
 *     RGBA8 textures must be tiled into 8x8 blocks (libctru helpers exist).
 *
 *   - Vertex format: citro3d takes interleaved attribute buffers. GX vertex
 *     descriptors translate directly; pre-bake into a fixed C3D_AttrInfo at
 *     init time and update VBO contents per draw.
 *
 *   - Coordinate system: PICA200 clips in [-1,1] like GL but the projection
 *     matrix produced by Mtx44OrthoTilt / Mtx44PerspTilt (citro3d helpers)
 *     bakes in the 90deg rotation the top screen needs. Use those instead
 *     of writing a vanilla projection matrix.
 *
 *   - EFB copy: GX copies the embedded framebuffer to main RAM for re-use as
 *     a texture (post-FX, mini-map, etc.). On PICA, render to an off-screen
 *     C3D_RenderTarget and bind it as a texture for the next pass.
 *
 * Concrete porting steps:
 *   1. Author n3ds_default.v.pica covering position/color/uv pass-through.
 *   2. Mirror pc_gx.c's PCGXState struct as N3DSGXState; replace GL state
 *      setters with citro3d equivalents (C3D_DepthTest, C3D_CullFace, etc).
 *   3. Replace pc_gx_texture.c texture upload with C3D_TexInit +
 *      C3D_TexLoadImage. Tile RGBA8 via GX_DisplayTransfer.
 *   4. Replace pc_gx_tev.c TEV setup with C3D_TexEnv configuration. Cap at
 *      6 stages; the rest needs a second draw call.
 *
 * None of the above is implemented yet. This file exists so the porting
 * surface is visible in the tree.
 */

#include "n3ds_platform.h"

/* TODO: Define N3DSGXState mirroring PCGXState from pc/include/pc_gx_internal.h
 * and implement the GX_* entry points the decomp expects (see
 * include/dolphin/gx/ for the API surface). */

void n3ds_gx_init(void)     { /* TODO */ }
void n3ds_gx_shutdown(void) { /* TODO */ }
