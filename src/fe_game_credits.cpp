#include "fe_game_credits.h"


#include "femultilinetext.h"
#include "ngl.h"
#include "panelfile.h"

#include "common.h"

VALIDATE_SIZE(fe_game_credits, 0x18u);

fe_game_credits::fe_game_credits()
{
    field_0 = false;
    field_8 = nullptr;
    field_10 = 0;
    field_14 = 0;
}


void fe_game_credits::Draw()
{
    if (!field_0)
        return;
    nglQuad quad;
    nglInitQuad(&quad);
    nglSetQuadRect(&quad,
                   -0.5f,
                   -0.5f,
                   static_cast<float>(nglGetScreenWidth()) + 0.5f,
                   static_cast<float>(nglGetScreenHeight()) + 0.5f);
    nglSetQuadZ(&quad, 999.0f);
    nglSetQuadColor(&quad, 0xFF000000);
    nglListAddQuad(&quad);
    reinterpret_cast<FEMultiLineText *>(field_4)->Draw();
    if (field_8 != nullptr)
        field_8->Draw();
}
