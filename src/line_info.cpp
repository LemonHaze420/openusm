#include "line_info.h"

#include "common.h"
#include "func_wrapper.h"
#include "oldmath_po.h"
#include "trace.h"
#include "utility.h"

#include <cassert>
#include <cmath>

VALIDATE_SIZE(line_info, 0x5C);


#if 0
simple_queue<line_info *, 16> & queued_collision_checks = var<simple_queue<line_info *, 16>>(0x009223F8);
#else
simple_queue<line_info *, 16> g_queued_collision_checks{};
simple_queue<line_info *, 16> &line_info::queued_collision_checks = g_queued_collision_checks;
#endif

line_info::line_info()
{
    this->hit_entity = {0};
    this->collision = false;
    this->field_59 = false;
    this->queued_for_collision_check = false;
    this->clear();
}

line_info::line_info(const vector3d &a2, const vector3d &a3) : line_info()
{
    this->field_0 = a2;
    this->field_C = a3;
}

line_info::~line_info()
{
    TRACE("line_info::~line_info");

    this->release_mem();
}

#ifndef TEST_CASE

int num_debug_line_info[2]{};

std::array<line_info[MAX_RENDERABLE_LINE_INFOS], 2> debug_line_info{};
#endif

void line_info::render(int num, bool a3)
{
    assert(num >= 0);

    assert(num < MAX_RENDERABLE_LINE_INFOS &&
           "If you need to render more line-info's edit line_info.cpp (Or reduce the usage!)");

    if (num < MAX_RENDERABLE_LINE_INFOS) {
        if (a3) {
            if (num > num_debug_line_info[0]) {
                num_debug_line_info[0] = num;
            }

            debug_line_info[0][num].copy(*this);

        } else {
            if (num > num_debug_line_info[1]) {
                num_debug_line_info[1] = num;
            }

            debug_line_info[1][num].copy(*this);
        }
    }
}

void line_info::clear()
{
    if constexpr (1) {
        this->collision = false;
        this->field_59 = false;
        this->hit_entity.field_0.field_0 = 0;

        this->hit_norm = ZVEC;
        this->hit_pos = ZEROVEC;
        this->field_30 = ZEROVEC;
        this->field_3C = ZVEC;
        this->m_obb = nullptr;
        if (this->queued_for_collision_check) {
            queued_collision_checks.find(this, 1);
            this->queued_for_collision_check = false;
        }
    } else {
        THISCALL(0x0048C9D0, this);
    }
}

bool line_info::check_collision(const local_collision::entfilter_base &p_ent_filter,
                                const local_collision::obbfilter_base &p_obb_filter, line_info_local_query *a4)
{
    TRACE("line_info::check_collision");

    if constexpr (0) {
        if (this->queued_for_collision_check) {
            this->remove_to_collision_check_queue();
        }

        if (this->field_59) {
            this->clear();
        }

        auto v1 = this->field_0;
        auto v2 = this->field_C;
        this->ent_filter = &p_ent_filter;
        this->obb_filter = &p_obb_filter;
        this->field_59 = true;
        this->collision = false;
        this->hit_pos = this->field_C;

        this->field_30 = this->field_C;
        this->hit_entity = {0};

        this->m_obb = nullptr;
        entity *v_hit_entity = nullptr;
        assert(ent_filter != nullptr && obb_filter != nullptr);

        auto v3 = v2 - v1;
        auto len = v3.length();
        if (len > 0.0) {
            auto iter_count = std::ceil(len / 99.999901);
            assert(iter_count > 0);

            vector3d v26 = v3 / iter_count;
            auto v25 = v1;
            for (auto i = 0; i < iter_count; ++i) {
                vector3d v22 = v25;
                v25 += v26;

                region *a7 = nullptr;
                this->collision = find_intersection(v22,
                                                    v25,
                                                    *this->ent_filter,
                                                    *this->obb_filter,
                                                    &this->hit_pos,
                                                    &this->hit_norm,
                                                    &a7,
                                                    &v_hit_entity,
                                                    &this->m_obb,
                                                    false);
                if (this->collision) {
                    assert(hit_pos.is_valid() && "line_info find_intersection failed");
                    assert(hit_norm.is_valid() && "line_info find_intersection failed");

                    this->hit_entity.field_0 =
                        (v_hit_entity != nullptr ? v_hit_entity->get_my_handle() : entity_base_vhandle{0});

                    break;
                }
            }
        }

        if (this->collision) {
            if (v_hit_entity == nullptr) {
                assert(hit_entity == INVALID_VHANDLE);

                this->field_30 = this->hit_pos;
                this->field_3C = this->hit_norm;
            } else {
                assert(hit_entity == v_hit_entity->get_my_vhandle());

                auto &abs_po = v_hit_entity->get_abs_po();
                this->field_30 = abs_po.inverse_xform(this->hit_pos);
                this->field_3C = abs_po.non_affine_inverse_xform(this->hit_norm);
            }
        }

        return this->collision;
    } else {
        return (bool)THISCALL(0x0052EE20, this, &p_ent_filter, &p_obb_filter, a4);
    }
}

bool line_info::remove_to_collision_check_queue()
{
    if constexpr (1) {
        auto result = queued_collision_checks.find(this, 1);
        this->queued_for_collision_check = false;
        return result;

    } else {
        bool(__fastcall * func)(void *) = CAST(func, 0x0052EE00);
        return func(this);
    }
}

bool line_info::release_mem()
{
    return this->remove_to_collision_check_queue();
}

void line_info::copy(const line_info &a2)
{
    if constexpr (0) {
    } else {
        THISCALL(0x006B6E00, this, &a2);
    }
}

void line_info::frame_advance(int a1)
{
    TRACE("line_info::frame_advance");

    CDECL_CALL(0x0052F120, a1);
}

void line_info::sub_48B410(Float a2)
{
    vector3d v4 = this->field_C - this->field_0;
    if (v4.length2() > a2 * a2) {
        v4.sub_48A850(a2);
        this->field_C = this->field_0 + v4;
    }
}

void line_info_patch()
{
    {
        FUNC_ADDRESS(address, &line_info::render);
        SET_JUMP(0x00519F00, address);
    }

    REDIRECT(0x005584CA, line_info::frame_advance);

    {
        FUNC_ADDRESS(address, &line_info::check_collision);
        //REDIRECT(0x, address);
    }
}
