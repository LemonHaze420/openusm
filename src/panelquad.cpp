#include "panelquad.h"

#include "common.h"
#include "from_mash_in_place_constructor.h"
#include "func_wrapper.h"
#include "mash_info_struct.h"
#include "mash_config.h"
#include "memory.h"
#include "matrix4x4.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "panelquadsection.h"
#include "panelfile.h"
#include "panelmeshsection.h"
#include "trace.h"
#include "vtbl.h"
#include "variables.h"

#include <algorithm>
#include <new>
#include <cmath>

VALIDATE_SIZE(PanelQuad, 0x4C);
namespace {
vector2d section_get_min(const PanelQuadSection &section)
{
    const auto &quad = section.field_14;
    vector2d result{
        quad.field_0[0].pos.field_0,
        quad.field_0[0].pos.field_4,
    };
    for (int vertex = 1; vertex < 4; ++vertex) {
        result[0] = std::max(result[0], quad.field_0[vertex].pos.field_0);
        result[1] = std::max(result[1], quad.field_0[vertex].pos.field_4);
    }
    return result;
}

vector2d section_get_max(const PanelQuadSection &section)
{
    const auto &quad = section.field_14;
    vector2d result{
        quad.field_0[0].pos.field_0,
        quad.field_0[0].pos.field_4,
    };
    for (int vertex = 1; vertex < 4; ++vertex) {
        result[0] = std::min(result[0], quad.field_0[vertex].pos.field_0);
        result[1] = std::min(result[1], quad.field_0[vertex].pos.field_4);
    }
    return result;
}
}  // namespace

PanelQuad::PanelQuad()
{
    if constexpr (STANDALONE_SYSTEM) {
        m_vtbl = 0x0087B990;
        field_10 = 4;
        field_4 = 1.0f;
        pqs.m_data = nullptr;
        pqs.m_max_size = 0;
        pqs.field_10 = true;
        pmesh = nullptr;
        field_34 = 0.0f;
        field_38 = 1.0f;
    } else {
        THISCALL(0x00637E80, this);
    }
}

PanelQuad::PanelQuad(from_mash_in_place_constructor *a2) : pqs(a2), field_3C(a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        field_10 = 4;
        field_4 = 1.0f;
        m_vtbl = 0x0087B990;
        (void)a2;
        field_3C.field_C = nullptr;
        if (pmesh != nullptr) {
            new (pmesh) PanelMeshSection();
            const tlFixedString mesh_name{field_3C.guts};
            pmesh->SetMesh(nglGetMeshInFile(mesh_name, PanelFile::g_curmeshfile));
        }
        field_34 = 0.0f;
        field_38 = 1.0f;
    } else {
        THISCALL(0x00637F00, this, a2);
    }
}

PanelQuad::PanelQuad(const char *a2) : field_3C(a2)
{
    this->m_vtbl = 0x0087B990;

    this->pqs.m_data = nullptr;
    this->pqs.m_max_size = 0;
    this->pqs.field_10 = true;
    this->field_34 = 0.0;
    this->field_14[0] = 0.0;
    this->pmesh = nullptr;
    this->field_38 = 1.0;
    this->field_14[1] = 0.0;
}

PanelQuad *__fastcall PanelQuad_constructor(PanelQuad *self, int, from_mash_in_place_constructor *a2)
{
    TRACE("PanelQuad::PanelQuad");
    if constexpr (STANDALONE_SYSTEM)
        return ::new (static_cast<void *>(self)) PanelQuad(a2);
    else
        return reinterpret_cast<PanelQuad *>(THISCALL(0x00637F00, self, a2));
}

PanelQuad::~PanelQuad()
{
#if STANDALONE_SYSTEM
    if (g_is_the_packer) {
        if (pqs.size() != 0 && pqs.at(0)->field_14.m_tex != nullptr)
            nglReleaseTexture(pqs.at(0)->field_14.m_tex);
        if (pmesh != nullptr)
            mem_dealloc(pmesh, sizeof(PanelMeshSection));
    }
    pqs.clear();
#else
    THISCALL(0x0043F7F0, this);
#endif
}

void *PanelQuad::operator new(size_t size)
{
    auto *mem = mem_alloc(size);
    return mem;
}

void PanelQuad::operator delete(void *ptr, size_t size)
{
    mem_dealloc(ptr, size);
}

void PanelQuad::_destruct_mashed_class()
{
    TRACE("PanelQuad::_destruct_mashed_class");

    THISCALL(0x00633A30, this);
}

void PanelQuad::_unmash(mash_info_struct *a1, void *a3)
{
    TRACE("PanelQuad::unmash");
    if constexpr (1) {
        a1->unmash_class_in_place(this->pqs, this);

        a1->unmash_class_in_place(this->field_3C, this);

#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
        uint8_t class_mashed = -1;
        [](mash_info_struct *a1, mash::buffer_type buffer, uint8_t &a3) {
            a3 = *a1->read_from_buffer(buffer, 1, 1);
        }(a1, mash::SHARED_BUFFER, class_mashed);
        assert(class_mashed == 0xAF || class_mashed == 0);
#endif

        if (this->pmesh != nullptr) {
            this->pmesh = (PanelMeshSection *)a1->read_from_buffer(
#if OPENUSM_XBOX_MASH_FORMAT
                mash::NORMAL_BUFFER,
#endif
                sizeof(PanelMeshSection),
                16);
        }
    } else {
        void(__fastcall * func)(void *, void *, mash_info_struct *, void *) = CAST(func, get_vfunc(m_vtbl, 0x4));
        func(this, nullptr, a1, a3);
    }
}

int PanelQuad::_get_mash_sizeof()
{
#if OPENUSM_XBOX_MASH_FORMAT && !defined(OPENUSM_XBPACK_V10)
    return 0x48;
#else
    return 0x4C;
#endif
}

vector2d PanelQuad::GetMax()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (pmesh != nullptr) {
            const auto *mesh = pmesh->field_40;
            if (mesh != nullptr) {
                return {
                    mesh->SphereCenter[0] - mesh->SphereRadius,
                    mesh->SphereCenter[1] - mesh->SphereRadius,
                };
            }
            return {0.0f, 0.0f};
        }

        vector2d result{0.0f, 0.0f};
        bool have_section = false;
        if (pqs.m_data != nullptr && pqs.m_size > 0) {
            for (int section_index = 0; section_index < pqs.m_size; ++section_index) {
                const auto *section = pqs.m_data[section_index];
                if (section == nullptr)
                    continue;

                const auto section_bounds = section_get_max(*section);
                if (!have_section) {
                    result = section_bounds;
                    have_section = true;
                } else {
                    result[0] = std::min(result[0], section_bounds[0]);
                    result[1] = std::min(result[1], section_bounds[1]);
                }
            }
        }
        return result;
    } else {
        vector2d result;
        THISCALL(0x00616990, this, &result);
        return result;
    }
}

vector2d PanelQuad::GetMin()
{
    if constexpr (STANDALONE_SYSTEM) {
        if (pmesh != nullptr) {
            const auto *mesh = pmesh->field_40;
            if (mesh != nullptr) {
                return {
                    mesh->SphereCenter[0] + mesh->SphereRadius,
                    mesh->SphereCenter[1] + mesh->SphereRadius,
                };
            }
            return {0.0f, 0.0f};
        }

        vector2d result{0.0f, 0.0f};
        bool have_section = false;
        if (pqs.m_data != nullptr && pqs.m_size > 0) {
            for (int section_index = 0; section_index < pqs.m_size; ++section_index) {
                const auto *section = pqs.m_data[section_index];
                if (section == nullptr)
                    continue;

                const auto section_bounds = section_get_min(*section);
                if (!have_section) {
                    result = section_bounds;
                    have_section = true;
                } else {
                    result[0] = std::max(result[0], section_bounds[0]);
                    result[1] = std::max(result[1], section_bounds[1]);
                }
            }
        }
        return result;
    } else {
        vector2d result;
        THISCALL(0x006168C0, this, &result);
        return result;
    }
}

void PanelQuad::sub_616710(Float a2, Float a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (pmesh != nullptr) {
            pmesh->field_0[3].x += a2;
            pmesh->field_0[3].y += a3;
        } else {
            for (int i = 0; i < pqs.size(); ++i) {
                auto *quad = bit_cast<nglQuad *>(&pqs.at(i)->field_14);
                for (int vertex = 0; vertex < 4; ++vertex)
                    nglSetQuadVPos(quad, vertex, quad->field_0[vertex].pos.x + a2, quad->field_0[vertex].pos.y + a3);
            }
        }
        field_14[0] += a2;
        field_14[1] += a3;
    } else {
        THISCALL(0x00616710, this, a2, a3);
    }
}

void PanelQuad::sub_616690(float *a2, float *a3)
{
    this->pqs.at(0)->sub_608EF0(a2, a3);
}

void PanelQuad::SetTexture(nglTexture *texture)
{
    if constexpr (STANDALONE_SYSTEM) {
        for (int i = 0; i < pqs.size(); ++i)
            nglSetQuadTex(reinterpret_cast<nglQuad *>(&pqs.at(i)->field_14), texture);
    } else {
        THISCALL(0x00616290, this, texture);
    }
}

void PanelQuad::_SetZvalueAbs(Float a2)
{
    this->field_8 = a2;

    auto *v3 = this->pmesh;
    if (v3 != nullptr) {
        v3->field_44 = a2;
        v3->field_0[3][2] = a2;
    }

    for (int i = 0; i < this->pqs.size(); ++i) {
        auto *q = &this->pqs.at(i)->field_14;
        nglSetQuadZ(bit_cast<nglQuad *>(q), a2);
    }
}

// 0x006160E0
void PanelQuad::Mask(float amount, int direction, float uv_extent)
{
    if (direction == 0 || pqs.empty())
        return;
    amount = std::clamp(amount, 0.0f, 1.0f);
    pqs.at(0)->Mask(amount, direction, uv_extent, field_38);
}

void PanelQuad::SetZvalueAbs(Float a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        _SetZvalueAbs(a2);
    } else {
        void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x38));
        func(this, nullptr, a2);
    }
}

void PanelQuad::Draw()
{
    TRACE("PanelQuad::Draw");
#if STANDALONE_SYSTEM
    if (!IsShown())
        return;
    if (pmesh != nullptr) {
        if (pmesh->field_48.get_alpha() == 0)
            return;
        auto transform = pmesh->field_0;
        nglMeshParams params{};
        params.Flags = 66;
        for (int axis = 0; axis != 3; ++axis) {
            const auto &row = transform[axis];
            const auto length = std::sqrt(double(row.x) * row.x + double(row.y) * row.y + double(row.z) * row.z);
            params.Scale[axis] = static_cast<float>(length);
            const auto inverse = 1.0 / length;
            transform[axis].x = static_cast<float>(row.x * inverse);
            transform[axis].y = static_cast<float>(row.y * inverse);
            transform[axis].z = static_cast<float>(row.z * inverse);
        }
        transform[3].x -= 0.47f;
        transform[3].y -= 0.47f;
        nglListAddMesh(pmesh->field_40, *reinterpret_cast<const math::MatClass<4, 3> *>(&transform), &params, nullptr);
        return;
    }
    for (int i = 0; i < pqs.size(); ++i) {
        auto *section = pqs.m_data[i];
        if (section->field_78)
            nglListAddQuad(reinterpret_cast<nglQuad *>(&section->field_14));
    }
#else
    THISCALL(0x00616090, this);
#endif
}

void PanelQuad::TurnOn(bool a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (a2)
            field_10 |= 4;
        else
            field_10 &= static_cast<char>(~4);
    } else {
        void(__fastcall * func)(void *, void *, bool) = CAST(func, get_vfunc(m_vtbl, 0x5C));
        func(this, nullptr, a2);
    }
}

void PanelQuad::Scale(Float a1, bool a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        float scale = equal(float(a1), 0.0f) ? 0.01f : float(a1);
        if (a2)
            scale /= field_38;
        for (int i = 0; i < pqs.size(); ++i)
            nglScaleQuad(bit_cast<nglQuad *>(&pqs.at(i)->field_14), field_14[0], field_14[1], scale, scale);
        field_38 *= scale;
    } else {
        void(__fastcall * func)(void *, void *, Float, bool) = CAST(func, get_vfunc(m_vtbl, 0x6C));
        func(this, nullptr, a1, a2);
    }
}

void PanelQuad::Rotate(Float a2, Float a3, Float a4, bool a5)
{
    TRACE("PanelQuad::Rotate");

    float a4a = (a5 ? a4 - this->field_34 : float(a4));

    for (int i = 0; i < this->pqs.size(); ++i) {
        nglRotateQuad(bit_cast<nglQuad *>(&this->pqs.m_data[i]->field_14), a2, a3, a4a);
    }

    this->field_34 += a4a;
}

void PanelQuad::SetColor(color32 a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        if (pmesh != nullptr)
            pmesh->field_48 = a2;
        const auto packed = static_cast<unsigned int>(color32::to_int(a2));
        for (int i = 0; i < pqs.size(); ++i)
            nglSetQuadColor(reinterpret_cast<nglQuad *>(&pqs.at(i)->field_14), packed);
    } else {
        void(__fastcall * func)(void *, void *, color32) = CAST(func, get_vfunc(m_vtbl, 0x7C));
        func(this, nullptr, a2);
    }
}

void PanelQuad::SetAlpha(Float a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        const float alpha = std::clamp(float(a2), 0.0f, 1.0f);
        if (pmesh != nullptr)
            pmesh->field_48.set_alpha(static_cast<uint8_t>(alpha * 255.0f));

        for (int section_index = 0; section_index < pqs.size(); ++section_index) {
            auto *section = pqs.at(section_index);
            auto *quad = reinterpret_cast<nglQuad *>(&section->field_14);
            for (int vertex = 0; vertex < 4; ++vertex) {
                const auto vertex_alpha = static_cast<uint8_t>(section->field_10[vertex]);
                const auto color = quad->field_0[vertex].m_color & 0x00FFFFFFu;
                const auto scaled_alpha = static_cast<uint32_t>(static_cast<float>(vertex_alpha) * alpha);
                nglSetQuadVColor(quad, vertex, color | (scaled_alpha << 24));
            }
        }
    } else {
        void(__fastcall * func)(void *, void *, Float) = CAST(func, get_vfunc(m_vtbl, 0x84));
        func(this, nullptr, a2);
    }
}

void PanelQuad::_GetCenterPos(float &a2, float &a3) const
{
    a2 = this->field_14[0];
    a3 = this->field_14[1];
}

void PanelQuad::GetCenterPos(float &a2, float &a3) const
{
    if constexpr (STANDALONE_SYSTEM) {
        a2 = field_14[0];
        a3 = field_14[1];
    } else {
        void(__fastcall * func)(const void *, void *, float *, float *) = CAST(func, get_vfunc(m_vtbl, 0xA8));
        func(this, nullptr, &a2, &a3);
    }
}

void PanelQuad::ResetToInitialXY()
{
    if constexpr (STANDALONE_SYSTEM) {
        for (int i = 0; i < pqs.size(); ++i) {
            auto *section = pqs.at(i);
            for (int vertex = 0; vertex < 4; ++vertex)
                nglSetQuadVPos(bit_cast<nglQuad *>(&section->field_14),
                               vertex,
                               section->field_0[vertex],
                               section->field_8[vertex]);
        }
        const auto minimum = GetMax();
        const auto maximum = GetMin();
        field_34 = 0.0f;
        field_38 = 1.0f;
        field_14[0] = (maximum[0] - minimum[0]) * 0.5f + minimum[0];
        field_14[1] = (maximum[1] - minimum[1]) * 0.5f + minimum[1];
    } else {
        THISCALL(0x00628840, this);
    }
}

void PanelQuad::SetCenterPos(Float x, Float y)
{
    if constexpr (STANDALONE_SYSTEM) {
        ResetToInitialXY();
        sub_616710(float(x) - field_14[0], float(y) - field_14[1]);
    } else {
        THISCALL(0x0043F6E0, this, x, y);
    }
}

color32 PanelQuad::_GetColor() const
{
    auto *v1 = this->pqs.at(0);
    return v1->GetColor(0);
}

color32 PanelQuad::GetColor() const
{
    if constexpr (STANDALONE_SYSTEM) {
        return _GetColor();
    } else {
        color32 result;
        void(__fastcall * func)(const void *, void *, color32 *) = CAST(func, get_vfunc(m_vtbl, 0xBC));
        func(this, nullptr, &result);
        return result;
    }
}

void PanelQuad::SetPos(float *a2, float *a3)
{
    if constexpr (STANDALONE_SYSTEM) {
        const auto target_min_x = *std::min_element(a2, a2 + 4);
        const auto target_max_x = *std::max_element(a2, a2 + 4);
        const auto target_min_y = *std::min_element(a3, a3 + 4);
        const auto target_max_y = *std::max_element(a3, a3 + 4);

        if (pqs.m_size == 1) {
            auto *section = pqs.m_data[0];
            for (int vertex = 0; vertex < 4; ++vertex) {
                nglSetQuadVPos(bit_cast<nglQuad *>(&section->field_14), vertex, a2[vertex], a3[vertex]);
            }
        } else if (pqs.m_size > 1) {
            auto current_min_x = pqs.m_data[0]->field_14.field_0[0].pos.field_0;
            auto current_max_x = current_min_x;
            auto current_min_y = pqs.m_data[0]->field_14.field_0[0].pos.field_4;
            auto current_max_y = current_min_y;
            for (int section_index = 0; section_index < pqs.m_size; ++section_index) {
                const auto *section = pqs.m_data[section_index];
                for (int vertex = 0; vertex < 4; ++vertex) {
                    const auto &pos = section->field_14.field_0[vertex].pos;
                    current_min_x = std::min(current_min_x, pos.field_0);
                    current_max_x = std::max(current_max_x, pos.field_0);
                    current_min_y = std::min(current_min_y, pos.field_4);
                    current_max_y = std::max(current_max_y, pos.field_4);
                }
            }

            const auto current_width = current_max_x - current_min_x;
            const auto current_height = current_max_y - current_min_y;
            const auto x_scale =
                (current_width > 0.0f || current_width < 0.0f) ? (target_max_x - target_min_x) / current_width : 1.0f;
            const auto y_scale = (current_height > 0.0f || current_height < 0.0f)
                                     ? (target_max_y - target_min_y) / current_height
                                     : 1.0f;
            for (int section_index = 0; section_index < pqs.m_size; ++section_index) {
                auto *section = pqs.m_data[section_index];
                for (int vertex = 0; vertex < 4; ++vertex) {
                    const auto &pos = section->field_14.field_0[vertex].pos;
                    nglSetQuadVPos(bit_cast<nglQuad *>(&section->field_14),
                                   vertex,
                                   (pos.field_0 - current_min_x) * x_scale + target_min_x,
                                   (pos.field_4 - current_min_y) * y_scale + target_min_y);
                }
            }
        }

        field_14[0] = (target_min_x + target_max_x) * 0.5f;
        field_14[1] = (target_min_y + target_max_y) * 0.5f;
    } else {
        void(__fastcall * func)(void *, void *, float *, float *) = CAST(func, get_vfunc(m_vtbl, 0x94));
        func(this, nullptr, a2, a3);
    }
}

void PanelQuad::SetPos(Float left, Float top, Float right, Float bottom)
{
    if constexpr (STANDALONE_SYSTEM) {
        float x[] = {left, right, left, right};
        float y[] = {top, top, bottom, bottom};
        SetPos(x, y);
    } else {
        void(__fastcall * func)(void *, void *, Float, Float, Float, Float) = CAST(func, get_vfunc(m_vtbl, 0x90));
        func(this, nullptr, left, top, right, bottom);
    }
}

void PanelQuad::Animate(const matrix4x4 &transform, Float visibility)
{
    const bool relative = (field_10 & 1) != 0;
    if (pmesh != nullptr) {
        pmesh->field_0 = transform;
        pmesh->field_0[0].z = 0.0f;
        pmesh->field_0[1].z = 0.0f;
        pmesh->field_0[2] = vector4d{0.0f, 0.0f, 1.0f, 0.0f};
        pmesh->field_0[3].z = field_8;
    } else {
        for (int i = 0; i < pqs.size(); ++i)
            pqs.at(i)->Animate(transform, field_8, relative);
    }
    const float alpha = relative ? float(visibility) * field_4 : float(visibility);
    SetAlpha(alpha);
    field_10 |= 1;
}

void PanelQuad::Init(vector2d *positions, color32 *colors, panel_layer layer, Float z, const char *name)
{
    if constexpr (STANDALONE_SYSTEM) {
        static vector2d uvs[] = {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {1.0f, 1.0f},
            {0.0f, 1.0f},
        };

        pqs.m_size = 1;
        pqs.m_max_size = 1;
        pqs.field_10 = true;
        pqs.m_data = static_cast<PanelQuadSection **>(mem_alloc(sizeof(*pqs.m_data)));
        auto *storage = mem_alloc(sizeof(PanelQuadSection));
        pqs.m_data[0] = ::new (storage) PanelQuadSection{};
        pqs.m_data[0]->Init(positions, uvs, colors, z);
        pmesh = nullptr;
        field_14 = {
            (positions[0][0] + positions[2][0]) * 0.5f,
            (positions[0][1] + positions[2][1]) * 0.5f,
        };
        field_3C = name != nullptr ? name : "";
        SetZvalue(z, layer);
    } else {
        void(__fastcall * func)(void *, void *, vector2d *, color32 *, panel_layer, Float, const char *) =
            CAST(func, get_vfunc(m_vtbl, 0x54));
        func(this, nullptr, positions, colors, layer, z, name);
    }
}

float PanelQuad::GetCenterX()
{
    TRACE("PanelQuad::GetCenterX");

    auto result = this->field_14[0];
    return result;
}

float PanelQuad::GetCenterY()
{
    TRACE("PanelQuad::GetCenterY");

    auto result = this->field_14[1];
    return result;
}

void PanelQuad::CopyFrom(const PanelQuad *a2)
{
    if constexpr (STANDALONE_SYSTEM) {
        field_4 = a2->field_4;
        field_8 = a2->field_8;
        field_C = a2->field_C;
        field_10 = a2->field_10;
        field_3C = a2->field_3C;
        field_34 = a2->field_34;
        field_38 = a2->field_38;

        pqs.m_size = a2->pqs.m_size;
        pqs.m_max_size = a2->pqs.m_size;
        pqs.field_10 = true;
        if (pqs.m_size > 0) {
            pqs.m_data = static_cast<PanelQuadSection **>(mem_alloc(sizeof(*pqs.m_data) * pqs.m_size));
            for (int i = 0; i < pqs.m_size; ++i) {
                pqs.m_data[i] = static_cast<PanelQuadSection *>(mem_alloc(sizeof(PanelQuadSection)));
                std::memcpy(pqs.m_data[i], a2->pqs.m_data[i], sizeof(PanelQuadSection));
            }
        } else {
            pqs.m_data = nullptr;
        }

        if (a2->pmesh != nullptr) {
            auto *storage = mem_alloc(sizeof(PanelMeshSection));
            pmesh = ::new (storage) PanelMeshSection{*a2->pmesh};
        } else {
            pmesh = nullptr;
        }
        field_14 = a2->field_14;
    } else {
        THISCALL(0x0062E130, this, a2);
    }
}

void PanelQuad::GetPos(float *a2, float *a3)
{
    TRACE("PanelQuad::GetPos");

    assert(pmesh == nullptr);

    for (auto i = 0u; i < 4u; ++i) {
        auto *quad_section = this->pqs.at(0);
        auto &quad = quad_section->field_14;
        a2[i] = quad.field_0[i].pos.field_0;
        a3[i] = quad.field_0[i].pos.field_4;
    }
}

void PanelQuad_patch()
{
    {
        FUNC_ADDRESS(address, &PanelQuad::Draw);
        set_vfunc(0x0087B9E8, address);
        set_vfunc(0x0087BAB8, address);
    }

    {
        FUNC_ADDRESS(address, &PanelQuad::GetCenterX);
        SET_JUMP(0x0043F770, address);
    }

    {
        FUNC_ADDRESS(address, &PanelQuad::GetCenterY);
        SET_JUMP(0x0043F780, address);
    }

    {
        FUNC_ADDRESS(address, &PanelQuad::Rotate);
        SET_JUMP(0x00616140, address);
    }

    {
        REDIRECT(0x00429C61, PanelQuad_constructor);
    }
    {
        FUNC_ADDRESS(address, &PanelQuad::_unmash);
        set_vfunc(0x0087B994, address);
    }

    {
        FUNC_ADDRESS(address, &PanelQuad::_destruct_mashed_class);
        set_vfunc(0x0087B990, address);
    }

    {
        FUNC_ADDRESS(address, &PanelQuad::GetPos);
        SET_JUMP(0x006166A0, address);
    }

    {
        FUNC_ADDRESS(address, &PanelQuad::_get_mash_sizeof);
        set_vfunc(0x0087B9DC, address);
    }
}
