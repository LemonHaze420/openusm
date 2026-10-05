#pragma once

struct FEMenuSystem;
struct PanelFile;

struct fe_game_credits {
    bool field_0;
    int field_4;
    PanelFile *field_8;
    int field_C;
    int field_10;
    char field_14;

    fe_game_credits();


    void Draw();
};
