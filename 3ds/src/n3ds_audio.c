/* n3ds_audio.c - ndsp audio backend (SCAFFOLD).
 *
 * PC equivalent: pc/src/pc_audio.c.
 *
 * The PC port runs a producer thread that calls pc_audio_process_frame()
 * into a ~512ms ring buffer, with an SDL audio callback as the consumer.
 *
 * On 3DS the equivalent is libctru's ndsp:
 *   - ndspInit() once at startup
 *   - reserve channel 0, configure NDSP_FORMAT_STEREO_PCM16 @ 32000 Hz
 *   - allocate two linear-RAM ndspWaveBuf buffers
 *   - producer thread fills whichever buffer ndspChnIsPlaying reports as
 *     done, then queues it with ndspChnWaveBufAdd
 *
 * Threading:
 *   - libctru threads: threadCreate(fn, arg, stack_sz, prio, processor, detach)
 *   - On Old 3DS only core 0 (AppCore) and core 1 (SysCore w/ APT_SetAppCpuTimeLimit)
 *     are usable; pin the audio producer to core 1.
 *   - LightLock / LightEvent / LightSemaphore replace SDL_atomic_ / SDL_mutex_.
 *
 * Sample rate: the decomp expects 32 kHz, which ndsp supports directly. No
 * resampling needed.
 */

#include "n3ds_platform.h"

/* TODO: ndsp init, ring buffer, producer thread, AIDMA callback dispatch.
 * Mirror the API surface of pc_audio_*. */

int  n3ds_audio_get_buffer_fill(void) { return 0; }
int  n3ds_audio_is_active(void)       { return 0; }
void n3ds_audio_set_paused(int paused){ (void)paused; }
void n3ds_audio_shutdown(void)        {}
void n3ds_audio_start_producer_thread(void) {}
void n3ds_audio_mq_init(void)         {}
void n3ds_audio_mq_shutdown(void)     {}
