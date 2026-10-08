#include "subtitles.h"

#include "fetext.h"
#include "game.h"
#include "localized_string_table.h"
#include "panelquad.h"
#include "variables.h"

namespace {
bool subtitles_initialized;
bool subtitles_enabled;
int subtitle_state;
FEText *subtitle_text;
PanelQuad *subtitle_backing;
float subtitle_duration;
float subtitle_delay;
float subtitle_elapsed;
int next_subtitle_id;
float next_subtitle_delay;
float next_subtitle_duration;
}  // namespace

void subtitles_init()
{
    subtitles_initialized = true;
    subtitles_enabled = false;
    subtitle_state = 0;

    subtitle_text = new FEText{
        static_cast<font_index>(1),
        static_cast<global_text_enum>(0),
        Float{320.0f},
        Float{415.0f},
        0,
        static_cast<panel_layer>(0),
        Float{1.0f},
        0,
        0,
        color32{0, 0, 0, 0},
    };
    subtitle_text->SetShown(true);

    subtitle_backing = new PanelQuad{};
    vector2d positions[] = {
        {120.0f, 400.0f},
        {520.0f, 400.0f},
        {120.0f, 430.0f},
        {520.0f, 430.0f},
    };
    color32 colors[] = {
        {0, 0, 0, 0xFF},
        {0, 0, 0, 0xFF},
        {0, 0, 0, 0xFF},
        {0, 0, 0, 0xFF},
    };
    subtitle_backing->Init(positions, colors, static_cast<panel_layer>(0), Float{1.0f}, "");
    subtitle_backing->TurnOn(true);
}

void subtitles_kill()
{
    subtitles_initialized = false;
    delete subtitle_text;
    subtitle_text = nullptr;
    delete subtitle_backing;
    subtitle_backing = nullptr;
}


void subtitles_enable()
{
    subtitles_enabled = true;
    subtitle_state = 0;
}


void subtitles_disable()
{
    subtitles_enabled = false;
}


void subtitles_set(int text_id, float delay, float duration, int next_text_id, float next_delay, float next_duration)
{
    if (subtitles_enabled) {
        const mString text{g_game_ptr->field_7C->lookup_scripttext_string(text_id)};
        subtitle_text->SetTextNoLocalize(FEText::string{text});
        subtitle_duration = duration;
        subtitle_delay = delay;
        next_subtitle_duration = next_duration;
        subtitle_elapsed = 0.0f;
        subtitle_state = 1;
        next_subtitle_id = next_text_id;
        next_subtitle_delay = next_delay;
    }
}


void subtitles_frame_advance(float time_inc)
{
    subtitle_elapsed += time_inc;
    if (subtitles_initialized && subtitles_enabled) {
        if (subtitle_state == 1) {
            if (subtitle_elapsed >= subtitle_delay)
                subtitle_state = 2;
        } else if (subtitle_state == 2 && subtitle_elapsed >= subtitle_duration) {
            if (next_subtitle_id != 0)
                subtitles_set(next_subtitle_id, next_subtitle_delay, next_subtitle_duration, 0, 0.0f, 0.0f);
            else
                subtitle_state = 0;
        }
    }
}


void subtitles_render()
{
    if (subtitles_enabled && globalTextLanguage != 0 && subtitle_text != nullptr && subtitle_state == 2) {
        subtitle_backing->Draw();
        subtitle_text->Draw();
    }
}
