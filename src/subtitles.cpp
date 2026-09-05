#include "subtitles.h"

#include "femultilinetext.h"
#include "panelquad.h"

#include <cstdio>
#include <cstdlib>

namespace {
bool subtitles_enabled;
bool subtitle_active;
int subtitle_state;
FEText *subtitle_text;
PanelQuad *subtitle_backing;
}

void subtitles_init()
{
    subtitles_enabled = true;
    subtitle_active = false;
    subtitle_state = 0;

    subtitle_text = new FEText {
        static_cast<font_index>(1),
        static_cast<global_text_enum>(0),
        Float {320.0f},
        Float {415.0f},
        0,
        static_cast<panel_layer>(0),
        Float {1.0f},
        0,
        0,
        color32 {0, 0, 0, 0},
    };
    subtitle_text->SetShown(true);

    subtitle_backing = new PanelQuad {};
    vector2d positions[] = {
        {120.0f, 400.0f},
        {520.0f, 400.0f},
        {520.0f, 430.0f},
        {120.0f, 430.0f},
    };
    color32 colors[] = {
        {0, 0, 0, 0xFF},
        {0, 0, 0, 0xFF},
        {0, 0, 0, 0xFF},
        {0, 0, 0, 0xFF},
    };
    subtitle_backing->Init(
        positions,
        colors,
        static_cast<panel_layer>(0),
        Float {1.0f},
        "");
    subtitle_backing->TurnOn(true);
}

void subtitles_kill()
{
    delete subtitle_backing;
    subtitle_backing = nullptr;
    delete subtitle_text;
    subtitle_text = nullptr;
    subtitle_active = false;
    subtitle_state = 0;
}
