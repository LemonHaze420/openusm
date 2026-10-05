#include "web_polytube.h"

#include "common.h"
#include "swinger.h"
#include "trace.h"
#include "utility.h"
#include "vtbl.h"
#include <array>

VALIDATE_SIZE(web_polytube, 0x17C);

namespace {
void *__fastcall native_destroy(web_polytube *self, void *, unsigned flags)
{
    self->~web_polytube();
    if (flags & 1)
        ::operator delete(self);
    return self;
}
void __fastcall native_render(web_polytube *self, void *, Float dt) { self->render(dt); }
void __fastcall native_rebuild(web_polytube *self, void *) { self->rebuild_web(); }
short __fastcall native_ifl_lock(web_polytube *self, void *, short frame)
{
    self->field_140 = frame;
    self->field_142 = 1;
    return frame;
}
void __fastcall native_ifl_play(web_polytube *self, void *) { self->field_142 = 0; }
void __fastcall native_ifl_pause(web_polytube *, void *) {}
short __fastcall native_ifl_frame(web_polytube *self, void *) { return self->field_140; }

std::intptr_t native_table(std::intptr_t inherited)
{
    static std::array<void *, 192> table{};
    if (!table[0]) {
        auto **base = reinterpret_cast<void **>(inherited);
        for (unsigned i = 0; i < table.size(); ++i)
            table[i] = base[i];
        table[0] = reinterpret_cast<void *>(&native_destroy);
        table[0x1AC / 4] = reinterpret_cast<void *>(&native_render);
        table[0x228 / 4] = reinterpret_cast<void *>(&native_ifl_lock);
        table[0x22C / 4] = reinterpret_cast<void *>(&native_ifl_play);
        table[0x230 / 4] = reinterpret_cast<void *>(&native_ifl_pause);
        table[0x234 / 4] = reinterpret_cast<void *>(&native_ifl_frame);
        table[0x238 / 4] = reinterpret_cast<void *>(&native_rebuild);
    }
    return reinterpret_cast<std::intptr_t>(table.data());
}
}

web_polytube::web_polytube(swinger_t *a1, const string_hash &a2, uint32_t a3) : polytube(a2, a3)
{
    this->m_vtbl = native_table(m_vtbl);
    this->field_178 = a1;
}

void web_polytube::render(Float dt)
{
    if (field_178)
        rebuild_web();
    polytube::_render(dt);
}

void web_polytube::rebuild_web()
{
    if constexpr (1) {
        auto *swinger = this->field_178;


        const vector3d &a2 = swinger->m_visual_point;
        auto &v38 = swinger->field_0;
        const vector3d v37 = swinger->field_3C->get_abs_position() + swinger->field_40;

        auto v2 = this->get_num_control_pts() - 1;
        this->set_abs_control_pt(v2, a2);
        this->set_abs_control_pt(0, v37);

        auto v4 = v37 * 3.0f + a2 * 2.0f;
        const auto v34 = v4 / 5.0f;
        auto v6 = v37 - v38.get_pivot_abs_pos();
        auto v18 = (v38.get_constraint() - v6.length() - 0.5f) / 2.0f;
        auto a3 = (v38.get_constraint() - 9.0f) / 9.0f + v18;
        if (a3 < LARGE_EPSILON) {
            a3 = 0.0f;
        }

        const auto v16 = YVEC * a3;
        auto v9 = (v37 + v34) / 2.0f - v16;
        this->set_abs_control_pt(1, v9);

        auto v12 = (a2 + v34) / 2.0f - v16;
        this->set_abs_control_pt(2, v12);
        this->rebuild_helper();

    } else {
        THISCALL(0x004775A0, this);
    }
}

void web_polytube_patch()
{
    {
        FUNC_ADDRESS(address, &web_polytube::render);
        //SET_JUMP(0x, address);
    }
}
