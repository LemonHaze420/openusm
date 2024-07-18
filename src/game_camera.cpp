#include "game_camera.h"

#include "collide.h"
#include "common.h"
#include "func_wrapper.h"
#include "local_collision.h"
#include "oldmath_po.h"
#include "osassert.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

#include <cmath>

VALIDATE_SIZE(game_camera::_camera_shake_t, 0x1Cu);
VALIDATE_SIZE(game_camera, 0x1A0u);

static Var<vector3d> chest {0x00960E3C};

void game_camera::_camera_shake_t::clear() {
    this->field_1A = 0;
    this->field_18 = -1;
    this->field_0 = 0;
    this->field_4 = 0;
    this->field_8 = 0;
    this->field_C = 0;
}

game_camera::_camera_shake_t::_camera_shake_t() {
    this->field_1A = 0;
    this->clear();
}

game_camera::game_camera(const string_hash &a2, [[maybe_unused]] entity *a3)
    : camera(nullptr, a2) {}

//FIXME 
entity *game_camera::get_target_entity() const
{
    TRACE("game_camera::get_target_entity");

    if constexpr (0)
    {
        auto *ent = this->field_118.get_volatile_ptr();
        if (ent != nullptr) {
            return ent;
        }

        return nullptr;
    }
    else
    {
        return (entity *) THISCALL(0x0057A220, this);
    }

}

void game_camera::set_target_entity(entity *e)
{
    TRACE("game_camera::set_target_entity");

    if (e != nullptr) {
        this->field_118 = {e->get_my_handle()};
    } else {
        this->field_118 = {0};
    }
}

void game_camera::frame_advance(Float t)
{
    assert(t >= 0 && t < 1e9f);

    if ( this->is_externally_controlled() )
    {
        this->field_12C = false;
    }
    else
    {
        auto *target_entity = this->get_target_entity();
        assert(target_entity != nullptr);

        this->field_11C = target_entity->get_abs_position();
        vector3d v26 = target_entity->get_abs_po().get_z_facing();

        auto v31 = this->get_abs_po();
        this->field_128 = this->field_11C[1];
        vector3d v27 = this->field_11C;

        if ( !this->field_11C.is_valid() )
        {
            warning("camera target went out of the world!!");
            this->field_11C = target_entity->get_abs_position();
        }

        bool v23 = target_entity->is_in_limbo();

        v27 += v26;

        vector3d v24;
        if ( v23 )
        {
            v24 = v31.get_position();
        }
        else
        {
            v24 = this->field_11C;
            v24[1] = this->field_128 + 0.69999999;

            auto &abs_po = target_entity->get_abs_po();
            auto v16 = abs_po.get_z_facing();
            v24 = v24 - (v16 * (3.0 - 1.0));
        }

        vector3d a2 = this->field_11C;
        a2[1] += s_camera_target_radius_factor;

        vector3d a5{};
        vector3d a6{};
        if ( !v23
                && find_intersection(
                    this->field_11C,
                    a2,
                    *local_collision::entfilter_line_segment_camera_collision,
                    *local_collision::obbfilter_lineseg_test,
                    &a5,
                    &a6,
                    nullptr,
                    nullptr,
                    nullptr,
                    false) )
        {
            a2 = a5;
            a2.y = a5.y - 0.30000001f;
            if ( v27[1] > a2[1] ) {
                v27[1] = a2[1];
            }

            if ( v24[1] > a2[1] ) {
                v24[1] = a2[1];
            }
        }

        chest() = this->field_11C;
        chest().y += 0.69999999f;
        if ( chest().y > a2.y ) {
            chest().y = a2.y;
        }

        if ( !v23
                && find_intersection(
                    this->field_11C,
                    v24,
                    *local_collision::entfilter_line_segment_camera_collision,
                    *local_collision::obbfilter_lineseg_test,
                    &a5,
                    &a6,
                    nullptr,
                    nullptr,
                    nullptr,
                    false) )
        {
            v24 = a6 * 0.30000001 + a5;
            if ( a6.y > -0.5f )
            {
                auto v21 = (v24 - v27).length2();
                if ( v21 > EPSILON ) {
                    v21 = std::sqrt(v21);
                    v24.y += s_camera_target_radius_factor - v21;
                }
            }
        }

        if ( v24.y > a2.y ) {
            v24.y = a2.y;
        }

        if ( !v23 )
        {
            vector3d v26 {0, 0, 0};
            auto *reg = this->get_primary_region();
            v24 = collide_with_world(this, v26, 0.30000001f, v24, reg);
        }

        assert(t >= 0 && t < 1e2f);

        this->blend(v27, v24, t);
    }
}

void game_camera::_sync(camera &a2)
{
    if ( !this->is_externally_controlled() )
    {
        camera::sync(a2);
        this->field_12C = false;
    }
}

void game_camera::blend(vector3d arg0, vector3d eax0, Float arg18)
{
    THISCALL(0x0057A330, this, arg0, eax0, arg18);
}

void game_camera_patch()
{
    {
        FUNC_ADDRESS(address, &game_camera::frame_advance);
        set_vfunc(0x00881CF4, address);
    }

    {
        FUNC_ADDRESS(address, &game_camera::set_target_entity);
        SET_JUMP(0x0057CC50, address);
    }
}
