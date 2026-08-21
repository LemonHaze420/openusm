#pragma once

#include "color.h"
#include "entity.h"
#include "float.hpp"
#include "variable.h"

struct generic_mash_header;
struct generic_mash_data_ptrs;
struct nglTexture;
struct vector3d;

enum class influence_type {
    POINT = 0,
    SPOT = 1,
    DIRECTIONAL = 2,
};

struct light_properties {
    influence_type m_type;
    color m_color;
    color m_amb_color;
    color m_add_color;
    float near_range;
    float cutoff_range;
    float field_3C;
    float near_hot;
    float cutoff_hot;
    float field_48;
    float m_contrast;
    int m_light_category;
    uint32_t m_flags;
    nglTexture *m_texture;
    vector3d m_scale;

    //0x00515890
    light_properties();

    //0x005602C0
    light_properties(const light_properties &) = default;

    influence_type get_type() const
    {
        return this->m_type;
    }


    //0x00515C70
    void un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);

    void set_light_category_from_light_source_name(const mString &a2);

    float get_influence(Float a2, Float a3) const;

    static constexpr auto default_contrast = 3.0f;
};

struct light_source : entity {
    light_properties *properties;

    light_source(const string_hash &a2, unsigned int a3);

    light_source(const light_properties &a2, entity *a3, const string_hash &a4);

    light_properties *get_properties();

    //virtual
    //0x00515E20
    void _un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4);

    //0x00515D50
    [[nodiscard]] vector3d get_dir(const vector3d &a3);

    void get_colors(const vector3d &a2, color &a3, color &a4);

    float get_influence(const vector3d &a2);

    float get_influence_directional(const vector3d &a2);

    float get_influence_spot(const vector3d &a2);

    float get_influence_point(const vector3d &a2);
};

struct USLightParam {
    light_source *field_0;

    static inline Var<int> ID{0x009566C4};
};
