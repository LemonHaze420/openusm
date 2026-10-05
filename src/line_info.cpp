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

line_info::line_info(from_mash_in_place_constructor *constructor)
    : field_0(constructor), field_C(constructor), hit_pos(constructor), hit_norm(constructor),
      field_30(constructor), field_3C(constructor)
{
    hit_entity.field_0 = {0};
}

line_info::line_info(const vector3d &a2, const vector3d &a3) : line_info()
{
    this->field_0 = a2;
    this->field_C = a3;
}

line_info::~line_info()
{
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

bool line_info::check_collision(const local_collision::entfilter_base &entity_filter,
                                const local_collision::obbfilter_base &terrain_filter, line_info_local_query *)
{
    if (queued_for_collision_check)
        remove_to_collision_check_queue();
    if (field_59)
        clear();
    const auto start = field_0;
    const auto end = field_C;
    ent_filter = &entity_filter;
    obb_filter = &terrain_filter;
    field_59 = true;
    collision = false;
    hit_pos = field_30 = end;
    hit_entity = {0};
    m_obb = nullptr;
    entity *hit = nullptr;
    const auto delta = end - start;
    const double length = std::sqrt(static_cast<double>(delta.x) * delta.x +
                                   static_cast<double>(delta.y) * delta.y +
                                   static_cast<double>(delta.z) * delta.z);
    if (length > 0.0) {
        const int count = static_cast<int>(std::ceil(length * 0.010000010021030903f));
        const auto step = delta / static_cast<float>(count);
        auto current = start;
        for (int i = 0; i != count; ++i) {
            const auto previous = current;
            current += step;
            region *hit_region = nullptr;
            collision = find_intersection(previous, current, entity_filter, terrain_filter,
                                          &hit_pos, &hit_norm, &hit_region, &hit, &m_obb, false);
            if (collision) {
                hit_entity.field_0 = hit != nullptr ? hit->get_my_handle() : entity_base_vhandle{0};
                break;
            }
        }
    }
    if (collision) {
        if (hit != nullptr) {
            const auto &transform = hit->get_abs_po();
            field_30 = transform.inverse_xform(hit_pos);
            field_3C = transform.non_affine_inverse_xform(hit_norm);
        } else {
            field_30 = hit_pos;
            field_3C = hit_norm;
        }
    }
    return collision;
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

void line_info::copy(const line_info &source)
{

    field_0 = source.field_0;
    field_C = source.field_C;
    hit_entity = source.hit_entity;
    collision = source.collision;
    hit_pos = source.hit_pos;
    hit_norm = source.hit_norm;
    field_30 = source.field_30;
    field_3C = source.field_3C;
    ent_filter = source.ent_filter;
    obb_filter = source.obb_filter;
    field_59 = source.field_59;
}

void line_info::frame_advance(int count)
{
    TRACE("line_info::frame_advance");

    while (count-- > 0 && queued_collision_checks.m_count > 0) {
        auto *line = queued_collision_checks.field_0[queued_collision_checks.field_4++];
        if (queued_collision_checks.field_4 >= queued_collision_checks.size) {
            queued_collision_checks.field_4 = 0;
        }
        --queued_collision_checks.m_count;
        line->queued_for_collision_check = false;
        line->check_collision(*line->ent_filter, *line->obb_filter, nullptr);
    }
}

void line_info::sub_48B410(Float a2)
{
    vector3d v4 = this->field_C - this->field_0;
    if (v4.length2() > a2 * a2) {
        v4.set_length(a2);
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
