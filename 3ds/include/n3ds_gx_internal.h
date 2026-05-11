/* n3ds_gx_internal.h - 3DS GX layer internal state.
 *
 * Mirrors pc/include/pc_gx_internal.h in shape: a single global state struct
 * holding everything the backend needs, plus init/shutdown and a minimal set
 * of draw entry points. We're starting tiny -- one VBO, one shader, one TEV
 * configuration, one draw test -- and will grow this surface as we wire in
 * the actual decomp GX entry points (GXSetViewport, GXSetVtxAttrFmt, etc.).
 */
#ifndef N3DS_GX_INTERNAL_H
#define N3DS_GX_INTERNAL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __3DS__
#include <3ds.h>
#include <citro3d.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* --- One-time GPU command buffer size ---
 * 0x40000 (256KB) matches the libctru default and roughly mirrors
 * pc_platform.h's PC_FIFO_SIZE. Bump later once real draws start landing. */
#define N3DS_GPU_CMDBUF_SIZE 0x40000

/* Vertex format for the test triangle. When we wire the decomp's vertex
 * descriptors (GXSetVtxDesc / GXSetVtxAttrFmt) this becomes a runtime layout,
 * but for now we have one fixed format. */
typedef struct {
    float pos[3];   /* attribute loader 0: GPU_FLOAT, 3 components */
    float clr[4];   /* attribute loader 1: GPU_FLOAT, 4 components */
} N3DSVertex;

typedef struct {
#ifdef __3DS__
    C3D_RenderTarget*  target_top;
    shaderProgram_s    program;
    DVLB_s*            vshader_dvlb;
    int                uloc_projection;
    C3D_Mtx            projection;
    void*              vbo;          /* linearAlloc'd, attribute buffer */
    int                vbo_capacity; /* in vertices */
#else
    int placeholder;
#endif
    int initialized;
} N3DSGXState;

extern N3DSGXState g_n3ds_gx;

/* --- Lifecycle --- */
int  n3ds_gx_init(void);     /* returns 1 on success */
void n3ds_gx_shutdown(void);

/* --- Frame --- */
void n3ds_gx_begin_frame(void);
void n3ds_gx_end_frame(void);

/* --- Test draws (will be removed when real GX entry points land) --- */
void n3ds_gx_draw_test_triangle(void);

#ifdef __cplusplus
}
#endif

#endif /* N3DS_GX_INTERNAL_H */
