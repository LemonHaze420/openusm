#pragma once

//0x005BC680
extern void subtitles_init();

extern void subtitles_kill();


extern void subtitles_enable();


extern void subtitles_disable();


extern void subtitles_set(int text_id, float delay, float duration, int next_text_id, float next_delay,
                          float next_duration);


extern void subtitles_frame_advance(float time_inc);


extern void subtitles_render();
