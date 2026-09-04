#pragma once

#include "color32.h"
#include "entity_base_vhandle.h"
#include "vector2d.h"
#include "vector3d.h"

struct camera;
struct hull;
struct mString;
struct nglVector;

struct motion_effect_struct {
    int field_0[11];
    char field_2C;

    //0x004DC560
    motion_effect_struct(entity_base_vhandle a2, const mString &a3);

    ~motion_effect_struct();

    static void render_all_motion_fx(camera &a2, hull &a3);

    //0x004DC820
    void render_trail(vector3d a1, vector3d a4, vector3d a7, vector2d a10, vector2d a12, vector2d a14, color32 a16,
                      color32 a17, color32 a18, bool a19, vector3d a20, vector3d a23);

    //0x004DCA10
    void render_distorted_trail(const vector3d &a1, const vector3d &a2, const vector3d &a3, const nglVector &a4,
                                const nglVector &a5, const nglVector &a6, color32 a7, color32 a8, color32 a9, bool a10,
                                vector3d &a11, vector3d &a12);

    //0x004EFA50
    static void record_all_motion_fx(Float a1);
};

extern void motion_effect_struct_patch();
