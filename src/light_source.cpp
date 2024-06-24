#include "light_source.h"

#include "fixedstring.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "oldmath_po.h"
#include "parse_generic_mash.h"
#include "trace.h"
#include "vector3d.h"
#include "variables.h"
#include "common.h"

VALIDATE_SIZE(light_properties, 0x68);
VALIDATE_SIZE(light_source, 0x6C);

light_properties::light_properties() : m_type(influence_type::POINT),
                                        m_color{1.0, 1.0, 1.0, 1.0},
                                        m_amb_color{1.0, 1.0, 1.0, 1.0},
                                        m_add_color{0, 0, 0, 0},
                                        m_scale{1.0, 1.0, 1.0}
{
    this->near_range = 1.0;
    this->cutoff_range = 16.0;

    this->near_hot = 1.0;
    this->cutoff_hot = 1.0e10;
    
    auto v1 = this->cutoff_range - this->near_range;
    this->field_3C = ( equal(v1, 0.0f) ? 1.0e10 : (1.f / v1) );

    auto v2 = this->cutoff_hot - this->near_hot;
    this->field_48 = ( equal(v2, 0.0f) ? 1.0e10 : (1.f / v2) );

    this->m_light_category = 1;
    this->m_flags = 0;
    this->m_texture = nullptr;
    this->m_contrast = light_properties::default_contrast;
}

void light_properties::un_mash(
        generic_mash_header *,
        void *,
        generic_mash_data_ptrs *a4)
{
    tlFixedString v6 = *a4->get_from_shared<tlFixedString>();

    if ( strlen(v6.field_4) != 0 )
    {
        tlFixedString v7 {v6.to_string()};
        this->m_texture = nglLoadTexture(v7);
    }
    else
    {
        this->m_texture = nullptr;
    }
}

void light_properties::set_light_category_from_light_source_name(const mString &a2)
{
    this->m_light_category = 0;
    
    auto v3 = a2.find("_CAT", 0);
    if ( v3 != -1 )
    {
        auto v4 = a2.c_str()[v3 + 4] - '0';
        switch ( v4 )
        {
          case 1:
            this->m_light_category = 32;
            break;
          case 2:
            this->m_light_category = 64;
            break;
          case 3:
            this->m_light_category = 128;
            break;
          case 4:
            this->m_light_category = 256;
            break;
        }
    }
}

float light_properties::get_influence(Float a2, Float a3) const
{
    if ( a2 <= this->near_range && a3 <= this->near_hot ) {
        return 1.0f;
    }

    if ( a2 < 0.0f || a2 >= this->cutoff_range || a3 >= this->cutoff_hot ) {
        return 0.0f;
    }

    float v4 = a2 - this->near_range;
    float v5 = std::max(v4, 0.0f);

    float v10 = a3 - this->near_hot;
    float v6 = std::max(v10, 0.0f);

    return (1.0f - this->field_48 * v6) * (1.0f - this->field_3C * v5);
}

light_source::light_source(const string_hash &a2, unsigned int a3) :
                        entity(a2, a3)
{

    this->m_vtbl = 0x00888C10;
    if ( !g_generating_vtables() )
    {
        this->properties = new light_properties{};

        auto *v6 = this->field_10.to_string();
        mString a2a {v6};
        this->properties->set_light_category_from_light_source_name(a2a);
    }
}

light_source::light_source(const light_properties &a2,
                        entity *a3,
                        const string_hash &a4) :
                        entity(a4, 0)
{
    this->m_vtbl = 0x00888C10;

    if ( !g_generating_vtables() )
    {
        this->properties = new light_properties(a2);

        auto *v7 = this->field_10.to_string();
        mString v10 {v7};
        this->properties->set_light_category_from_light_source_name(v10);
        if ( a3 != nullptr )
        {
            this->set_parent(a3);
        }
    }
}

light_properties *light_source::get_properties()
{
    assert(properties != nullptr);
    return this->properties;
}

void light_source::_un_mash(
        generic_mash_header *header,
        void *a3,
        generic_mash_data_ptrs *a4)
{
    entity::un_mash(header, a3, a4);

    a4->rebase(4u);

    this->properties = a4->get<light_properties>();

    assert(((int)header) % 4 == 0);

    this->properties->un_mash(header, this->properties, a4);
}

void light_source::get_colors(const vector3d &a2, color &a3, color &a4)
{
    TRACE("light_source::get_colors");

    if constexpr (0)
    {
        auto influence = this->get_influence(a2);
        a3 = this->get_properties()->m_color * influence;
        
        color new_color {1.0, 1.0, 1.0, 1.0};
        color a3a = new_color * (1.0f - influence);

        color v14 = this->properties->m_amb_color * influence;
        a4 = v14 + a3a;
    }
    else
    {
        THISCALL(0x0041D050, this, &a2, &a3, &a4);
    }
}

vector3d light_source::get_dir(const vector3d &a3)
{
    vector3d v7;

    if (this->properties->get_type() == influence_type::DIRECTIONAL)
    {
        po &abs_po = this->get_abs_po();

        vector3d v5 = abs_po.get_y_facing();

        v7 = -v5;
    }
    else
    {
        v7 = a3 - this->get_abs_position();
        v7.normalize();
    }

    return v7;
}

float light_source::get_influence(const vector3d &a2)
{
    auto v2 = this->properties->get_type();
    switch ( v2 )
    {
    case influence_type::POINT:
        return this->get_influence_point(a2);
        break;
    case influence_type::SPOT:
        return this->get_influence_spot(a2);
        break;
    case influence_type::DIRECTIONAL:
        return this->get_influence_directional(a2);
        break;
    default:
        break;
    }

    return 0.0f;
}

float light_source::get_influence_point(const vector3d &a2)
{
    auto abs_pos = this->get_abs_position();
    auto v7 = abs_pos - a2;
    auto a2a = v7.length();
    return this->properties->get_influence(a2a, -1000.0);
}

float light_source::get_influence_directional(const vector3d &a2)
{
    auto abs_pos = this->get_abs_position();
    auto v1 = abs_pos - a2;

    auto y_facing = this->get_abs_po().get_y_facing();
    auto a2a = dot(v1, y_facing);

    auto a3 = vector3d::cross(v1, y_facing).length();
    return this->properties->get_influence(a2a, a3);
}

float light_source::get_influence_spot(const vector3d &a2)
{
    auto abs_pos = this->get_abs_position();
    vector3d v4 = a2 - abs_pos;
    auto a2a = v4.length();

    auto y_facing = this->get_abs_po().get_y_facing();
    auto a3 = dot(v4, y_facing);
    return this->properties->get_influence(a2a, a3);
}
