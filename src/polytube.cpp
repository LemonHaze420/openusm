#include "polytube.h"

#include "common.h"
#include "func_wrapper.h"
#include "memory.h"
#include "oldmath_po.h"
#include "us_pcuv_shader.h"
#include "slab_allocator.h"
#include "string_hash.h"
#include "trace.h"
#include "vtbl.h"
#include "time_interface.h"
#include "variable.h"
#include "wds.h"
#include "polytubecustommaterial.h"
#include "ngl_mesh.h"
#include "ngl_scene.h"
#include "ngl_dx_vertexdef.h"
#include "tl_system.h"
#include "tl_instance_bank.h"
#include "entity_mash.h"
#include "moved_entities.h"
#include "game.h"
#include "camera.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <functional>

#include <cassert>

VALIDATE_SIZE(polytube, 0x178u);
VALIDATE_SIZE(polytube_pt_anim, 0x2C);

VALIDATE_SIZE(PolytubeCustomVertex::Iterator, 0x4Cu);
static Var<polytube *> active_polytubes{0x00965F50};
static Var<polytube *> inactive_polytubes{0x00965F4C};

namespace {
struct TentacleVertex {
    vector3d position;
    vector2d uv;
    uint32_t color;
    vector3d normal;
};
VALIDATE_SIZE(TentacleVertex, 0x24);

struct TentacleMeshIterator : nglVertexDef::IteratorBase {
    nglVertexDef *definition;
    unsigned index;

    TentacleMeshIterator(nglVertexDef *definition, bool lock);
    void *destroy(unsigned char flags)
    {
        if (flags & 1)
            operator delete(this);
        return this;
    }
    TentacleMeshIterator *clone() const
    {
        return new TentacleMeshIterator(*this);
    }
    bool has_next() const
    {
        return index < static_cast<unsigned>(definition->field_4->NVertices);
    }
    void write(const vector3d &position, const vector3d &normal, uint32_t color, float u, float v)
    {
        auto *section = definition->field_4;
        auto *vertices = reinterpret_cast<TentacleVertex *>(section->field_3C.getVertexData() + section->field_4C);
        vertices[index++] = {position, {u, v}, color, normal};
    }
};
VALIDATE_SIZE(TentacleMeshIterator, 0xC);

TentacleMeshIterator::TentacleMeshIterator(nglVertexDef *definition, bool lock) : definition(definition), index(0)
{
    static void *table[]{func_address(&TentacleMeshIterator::destroy),
                         func_address(&TentacleMeshIterator::clone),
                         func_address(&TentacleMeshIterator::has_next)};
    m_vtbl = reinterpret_cast<std::intptr_t>(table);
    if (lock && (definition->field_4->Flags & NGLMESH_TEMP) == 0) {
        void *data;
        IDirect3DVertexBuffer9_Lock(definition->field_4->field_3C.getVertexBuffer(), 0, 0, &data, 0);
        definition->field_4->field_3C.setVertexData(static_cast<char *>(data));
    }
}

void __fastcall tentacle_rebase(nglVertexDef *self, void *, int offset)
{
    self->field_4 = reinterpret_cast<nglMeshSection *>(reinterpret_cast<char *>(self->field_4) + offset);
}
TentacleMeshIterator **__fastcall tentacle_edit(nglVertexDef *self, void *, TentacleMeshIterator **out)
{
    *out = new TentacleMeshIterator(self, false);
    return out;
}
nglVertexDef *__fastcall tentacle_copy(nglVertexDef *, void *, nglMeshSection *);
void __fastcall tentacle_destroy(nglVertexDef *self, void *)
{
    tlMemFree(self);
}

void __fastcall tentacle_apply_morph(nglVertexDef *, void *, nglMorphSetSection *, uint32_t, Float) {}

nglVertexDef *process_tentacle(nglVertexDef *self)
{
    static void *table[]{reinterpret_cast<void *>(tentacle_rebase),
                         reinterpret_cast<void *>(tentacle_edit),
                         reinterpret_cast<void *>(tentacle_copy),
                         reinterpret_cast<void *>(tentacle_destroy),
                         reinterpret_cast<void *>(tentacle_apply_morph)};
    self->m_vtbl = reinterpret_cast<std::intptr_t>(table);
    return self;
}
nglVertexDef *__fastcall tentacle_copy(nglVertexDef *, void *, nglMeshSection *section)
{
    auto *result = process_tentacle(static_cast<nglVertexDef *>(tlMemAlloc(sizeof(nglVertexDef), 8, 0)));
    result->field_4 = section;
    return result;
}

void close_vertices(nglVertexDef *definition)
{
    auto *section = definition->field_4;
    if ((section->Flags & NGLMESH_TEMP) == 0)
        IDirect3DVertexBuffer9_Unlock(section->field_3C.getVertexBuffer());
}

struct TentacleCustomIterator {
    int point_index;
    unsigned point_count;
    PolytubeCustomOffset::Iterator *offsets;
    TentacleMeshIterator vertices;
    vector3d previous_point;
    vector3d previous_up;
    vector3d previous_right;
    float previous_radius;
    float previous_v;
    uint32_t color;
    float tiles;
    float phase;

    static nglVertexDef *create(unsigned count, Tentacle_ShaderMaterial *material,
                                PolytubeCustomOffset::Iterator *offsets)
    {
        nglCreateMesh(NGLMESH_TEMP, 1, 0, nullptr);
        auto *definition = nglCreateTentacleVertexDef();
        nglVertexDef_MultipassMesh_Base::AddMeshSection(definition,
                                                        material->material(),
                                                        2 * offsets->field_8 * (count - 1),
                                                        count - 1,
                                                        0,
                                                        nullptr,
                                                        36,
                                                        D3DPT_TRIANGLESTRIP,
                                                        true);
        return definition;
    }
    TentacleCustomIterator(unsigned count, Tentacle_ShaderMaterial *material, PolytubeCustomOffset::Iterator *offsets,
                           uint32_t color, float tiles, float phase)
        : point_index(0), point_count(count), offsets(offsets), vertices(create(count, material, offsets), true),
          color(color), tiles(tiles), phase(phase)
    {}

    void write(const vector3d &point, float radius)
    {
        float v = phase;
        vector3d up{0, 0, 1};
        vector3d right;
        offsets->field_4 = 0;
        if (point_index > 0) {
            vector3d direction = point - previous_point;
            const float length = direction.length();
            direction = direction / length;
            if (point_index == 1) {
                previous_right = vector3d::cross(direction, previous_up);
                previous_right.normalize();
                previous_up = vector3d::cross(previous_right, direction);
                previous_v = phase;
            }
            right = vector3d::cross(direction, previous_up);
            right.normalize();
            up = vector3d::cross(right, direction);
            v = length * tiles + previous_v;
            const float step = 1.0f / (offsets->field_8 - 1);
            float u = 0;
            BeginStrip(2 * offsets->field_8, sizeof(TentacleVertex));
            for (; offsets->field_4 < static_cast<int>(offsets->field_8); ++offsets->field_4) {
                const vector2d &offset = offsets->field_0[offsets->field_4];
                const vector3d previous = previous_point + offset.x * previous_radius * previous_right +
                                          offset.y * previous_radius * previous_up;
                const vector3d current = point + offset.x * radius * right + offset.y * radius * up;
                vector3d normal = previous - previous_point;
                if (normal.length2() > 9.999999439624929e-11f)
                    normal = normal / std::sqrt(normal.length2());
                vertices.write(previous, normal, color, u, previous_v);
                normal = current - point;
                if (normal.length2() > 9.999999439624929e-11f)
                    normal = normal / std::sqrt(normal.length2());
                vertices.write(current, normal, color, u, v);
                u += step;
            }
        }
        previous_point = point;
        previous_up = up;
        previous_right = right;
        previous_radius = radius;
        previous_v = v;
        ++point_index;
    }
};
VALIDATE_SIZE(TentacleCustomIterator, 0x50);

const vector3d *skip_coincident_points(const vector3d *point, const vector3d *end)
{
    while (point + 1 != end && (*point - point[1]).length2() < .0001f)
        ++point;
    return point;
}
}  // namespace

nglVertexDef *nglCreateTentacleVertexDef()
{
    return process_tentacle(static_cast<nglVertexDef *>(nglMeshAllocFn()(sizeof(nglVertexDef), 4, 0)));
}

void nglRegisterTentacleVertexDef()
{
    nglVertexDefBank.Insert(tlFixedString("Tentacle"), reinterpret_cast<void *>(process_tentacle));
}

polytube_pt_anim::polytube_pt_anim()
    : field_0(0), field_4(ZEROVEC), field_10(ZEROVEC), field_1C(0.0), field_20(0.0), field_24(0.0), field_28(0.0)
{}


void polytube_pt_anim::set_anim(const vector3d &start, const vector3d &direction, float duration, unsigned flags)
{
    if (duration <= 0.0f) {
        field_0 &= ~1u;
        return;
    }
    field_0 = flags;
    field_4 = start;
    double length;
    if (flags & 8) {
        field_1C = direction.x;
        constexpr double random_scale = 3.0518509447574615e-05;
        const float z = 2.0 * (std::rand() * random_scale);
        const float y = 2.0 * (std::rand() * random_scale);
        const double x = 2.0 * (std::rand() * random_scale);
        field_10 = vector3d{static_cast<float>((x - 1.0) * field_1C),
                            static_cast<float>((static_cast<double>(y) - 1.0) * field_1C),
                            static_cast<float>((static_cast<double>(z) - 1.0) * field_1C)};
        length = std::sqrt(static_cast<double>(field_10.x) * field_10.x + static_cast<double>(field_10.y) * field_10.y +
                           static_cast<double>(field_10.z) * field_10.z);
    } else {
        field_10 = direction;
        length = std::sqrt(static_cast<double>(field_10.z) * field_10.z + static_cast<double>(field_10.y) * field_10.y +
                           static_cast<double>(field_10.x) * field_10.x);
        field_1C = length;
    }
    if (length > 0.0) {
        const double inverse_length = 1.0 / length;
        field_10.x = inverse_length * field_10.x;
        field_10.y = inverse_length * field_10.y;
        field_10.z = inverse_length * field_10.z;
    }
    field_20 = duration;
    field_24 = duration;
    field_0 |= 1;
    field_28 = ((flags & 8) ? length : static_cast<double>(field_1C)) / duration;
}

void polytube_pt_anim::frame_advance(Float elapsed, vector3d &point)
{
    float dt = elapsed;
    if (dt <= field_24)
        field_24 -= dt;
    else {
        dt = field_24;
        field_24 = 0;
    }
    point = dt * field_28 * field_10 + point;
    if (field_24 <= 0) {
        if (field_0 & 6) {
            if (field_0 & 8) {
                constexpr float random_scale = 3.0518509447574615e-05f;
                const float z = 2.0f * (std::rand() * random_scale);
                const float y = 2.0f * (std::rand() * random_scale);
                const float x = 2.0f * (std::rand() * random_scale);
                field_10 = vector3d{(x - 1) * field_1C + field_4.x,
                                    (y - 1) * field_1C + field_4.y,
                                    (z - 1) * field_1C + field_4.z} -
                           point;
                const float length = field_10.length();
                if (length > 0)
                    field_10 = field_10 / length;
                field_28 = length / field_20;
                field_24 = field_20;
            } else {
                field_10 = -field_10;
                field_24 = (field_0 & 4) ? field_20 + field_20 : field_20;
            }
        } else {
            field_0 &= ~1u;
        }
    }
}

namespace {
color32 *__fastcall tube_color(const polytube *self, void *, color32 *out)
{
    *out = self->get_render_color();
    return out;
}
vector3d *__fastcall tube_center(polytube *self, void *, vector3d *out)
{
    *out = self->get_visual_center();
    return out;
}
int __fastcall tube_flavor(polytube *, void *)
{
    return 17;
}
void __fastcall tube_alpha(polytube *self, void *, float value)
{
    self->field_12C = value;
}
float __fastcall tube_get_alpha(polytube *self, void *)
{
    return self->field_12C;
}

std::intptr_t polytube_table(std::intptr_t inherited)
{
    static std::array<void *, 192> table{};
    if (table[0x1AC / 4] == nullptr) {
        std::copy_n(reinterpret_cast<void **>(inherited), table.size(), table.begin());
        table[0] = func_address(&polytube::destroy);
        table[0x28 / 4] = func_address(&polytube::get_visual_radius);
        table[0x2C / 4] = reinterpret_cast<void *>(tube_center);
        table[0x44 / 4] = func_address(&polytube::set_visible);
        table[0x54 / 4] = reinterpret_cast<void *>(tube_flavor);
        table[0x1A4 / 4] = func_address(&polytube::frame_advance);
        table[0x1AC / 4] = func_address(&polytube::_render);
        table[0x1C0 / 4] = func_address(&polytube::set_render_color);
        table[0x1C4 / 4] = reinterpret_cast<void *>(tube_color);
        table[0x1C8 / 4] = reinterpret_cast<void *>(tube_alpha);
        table[0x1CC / 4] = reinterpret_cast<void *>(tube_get_alpha);
        table[0x210 / 4] = func_address(&polytube::set_tentacle_width);
        table[0x214 / 4] = func_address(&polytube::get_tentacle_width);
        table[0x218 / 4] = func_address(&polytube::set_tentacle_activity);
        table[0x21C / 4] = func_address(&polytube::get_tentacle_activity);
        table[0x220 / 4] = func_address(&polytube::set_tentacle_pull_factor);
        table[0x224 / 4] = func_address(&polytube::get_tentacle_pull_factor);
        table[0x228 / 4] = func_address(&polytube::ifl_lock);
        table[0x22C / 4] = func_address(&polytube::ifl_play);
    }
    return reinterpret_cast<std::intptr_t>(table.data());
}
}  // namespace

polytube::polytube(const string_hash &a2, uint32_t a3) : entity(a2, a3)
{
    static Var<bool> g_generating_vtables = (0x0095A6F1);
    m_vtbl = polytube_table(m_vtbl);

    this->field_79 = 0;
    this->field_7A = 0;
    this->field_7B = 0;
    this->field_11C = 0;
    this->field_120 = 0;
    this->field_128 = 0;
    misc_render_objects.clear();
    if (!g_generating_vtables()) {
        this->field_68 = nullptr;
        this->field_6C = nullptr;
        this->init();
    }
}

polytube::~polytube()
{
    static Var<bool> generating_vtables{0x0095A6F1};
    if (!generating_vtables()) {
        const auto release = [](auto *&material) {
            if (material) {
                material->destroy(0);
                mem_dealloc(material, sizeof(*material));
                material = nullptr;
            }
        };
        release(field_D0);
        release(field_D8);
        release(field_D4);
        release(field_E4);
        destroy_offsets();
        destroy_tentacle_info();
        remove_from_list();
    }
    misc_render_objects.clear();
}

void *polytube::destroy(unsigned char flags)
{
    this->~polytube();
    if (flags & 1)
        operator delete(this);
    return this;
}

void polytube::remove_from_list()
{
    if (field_6C)
        field_6C->field_68 = field_68;
    else if (inactive_polytubes() == this)
        inactive_polytubes() = field_68;
    else if (active_polytubes() == this)
        active_polytubes() = field_68;
    if (field_68)
        field_68->field_6C = field_6C;
    field_68 = field_6C = nullptr;
}

void polytube::update_active_list()
{
    remove_from_list();
    const bool animated = std::any_of(
        pt_anims.begin(), pt_anims.end(), [](const polytube_pt_anim &anim) { return (anim.field_0 & 1) != 0; });
    auto &head = field_130 || ((field_4 & 0x200) && (std::not_equal_to<float>{}(field_100, 0.0f) || animated))
                     ? active_polytubes()
                     : inactive_polytubes();
    field_68 = head;
    head = this;
    if (field_68)
        field_68->field_6C = this;
}

void polytube::set_visible(bool visible, bool include_children)
{
    entity::_set_visible(visible, include_children);
    update_active_list();
}

void polytube::frame_advance(Float dt)
{
    if (std::not_equal_to<float>{}(field_100, 0.0f)) {
        field_104 += static_cast<float>(dt) * field_100;
        if (field_104 > 1)
            field_104 -= std::floor(field_104);
        if (field_104 < -1)
            field_104 += std::floor(std::fabs(field_104));
    }
    auto *point = the_spline.control_pts.begin();
    for (auto &anim : pt_anims) {
        if (anim.field_0 & 1) {
            anim.frame_advance(dt, *point);
            the_spline.need_rebuild = true;
        }
        ++point;
    }
    if (field_130) {
        field_130->frame_advance(dt);
        field_130->update_spline();
    }
}

vector3d polytube::get_visual_center()
{
    rebuild_helper();
    return get_abs_po().slow_xform(the_spline.field_44);
}

float polytube::get_visual_radius()
{
    rebuild_helper();
    const auto scale = entity::get_render_scale();
    return the_spline.field_40 * std::max(std::fabs(scale.x), std::max(std::fabs(scale.y), std::fabs(scale.z)));
}

void polytube::simulate_slack(const vector3d &start, const vector3d &end, float length)
{
    entity_set_abs_position(this, start);
    set_abs_control_pt(0, start);
    const int count = get_num_control_pts();
    set_abs_control_pt(count - 1, end);
    vector3d direction = start - end;
    const float distance = direction.length();
    const float slack = length - distance;
    if (distance > .0001f)
        direction = direction / distance;
    const float interval = distance / (count - 1);
    const int inner_points = count - 2;
    if (slack > .1f && inner_points > 0) {
        const float sag_step = slack * .5f / ((count - 3) * .5f + 1);
        const int midpoint = 1 - static_cast<int>((count - 3) * -.5f);
        float sag = sag_step;
        for (int point = inner_points, step = 1; point > 0; --point, ++step) {
            vector3d position = step * interval * direction + end;
            position.y -= sag;
            set_abs_control_pt(point, position);
            if (point <= midpoint)
                sag = std::max(0.0f, sag - sag_step);
            else
                sag += sag_step;
        }
    } else {
        for (int point = inner_points, step = 1; point > 0; --point, ++step)
            set_abs_control_pt(point, step * interval * direction + end);
    }
    rebuild_helper();
    moved_entities::add_moved(vhandle_type<entity>{my_handle});
}

void polytube::kill_anim(int index, bool restore_position)
{
    if (index >= 0) {
        if (static_cast<unsigned>(index) < pt_anims.size() &&
            static_cast<unsigned>(index) < the_spline.control_pts.size()) {
            auto &anim = pt_anims[index];
            if (restore_position)
                the_spline.control_pts[index] = anim.field_4;
            anim.field_0 &= ~1u;
        }
    } else {
        const auto count = std::min(pt_anims.size(), the_spline.control_pts.size());
        for (unsigned i = 0; i < count; ++i) {
            if (restore_position)
                the_spline.control_pts[i] = pt_anims[i].field_4;
            pt_anims[i].field_0 &= ~1u;
        }
        _std::vector<polytube_pt_anim> empty;
        pt_anims.swap(empty);
    }
    update_active_list();
}

void polytube::clear_simulations()
{
    kill_anim(-1, false);
    destroy_tentacle_info();
    field_11C = field_120 = INVALID_HANDLE;
    _std::vector<vector3d> controls, curve;
    the_spline.control_pts.swap(controls);
    the_spline.curve_pts.swap(curve);
    the_spline.need_rebuild = true;
}

int polytube::get_num_control_pts()
{
    return this->the_spline.get_num_control_pts();
}

void polytube::build(int a1, spline::eSplineType a2)
{
    this->the_spline.build(a1, a2);
}

PolytubeCustomOffset::Iterator::Iterator(unsigned count)
    : field_0(static_cast<vector2d *>(operator new(sizeof(vector2d) * count))), field_4(0), field_8(count)
{}

PolytubeCustomOffset::Iterator::~Iterator()
{
    operator delete[](field_0);
}

void polytube::destroy_offsets()
{
    if (field_74) {
        field_74->~Iterator();
        mem_dealloc(field_74, sizeof(*field_74));
        field_74 = nullptr;
    }
    if (field_70) {
        field_70->~Iterator();
        mem_dealloc(field_70, sizeof(*field_70));
        field_70 = nullptr;
    }
}

void polytube::init_offsets()
{
    assert(num_sides > 0);
    assert(tube_radius > 0);
    if (field_7B)
        num_sides = 12;
    auto *&offsets = field_E4 ? field_70 : field_74;
    const unsigned count = num_sides > 2 ? num_sides + 1 : 2;
    offsets = new (mem_alloc(sizeof(*offsets))) PolytubeCustomOffset::Iterator(count);
    for (unsigned i = 0; i < count; ++i) {
        if (num_sides > 2) {
            const double angle = static_cast<double>(i) * 6.283180236816406f / num_sides;
            offsets->field_0[i] = {
                static_cast<float>(std::cos(static_cast<double>(static_cast<float>(angle))) * tube_radius),
                static_cast<float>(std::sin(angle) * tube_radius)};
        } else {
            offsets->field_0[i] = {(i == 0 ? -.5f : .5f) * tube_radius, 0};
        }
        ++offsets->field_4;
    }
    field_78 = 1;
}

void polytube::init()
{
    this->field_D0 = nullptr;
    this->field_D8 = nullptr;
    this->field_D4 = nullptr;
    this->field_E4 = nullptr;
    this->num_sides = 2;
    this->tube_radius = 0.025f;
    this->tiles_per_meter = 1.0;
    this->field_79 = 1;
    this->field_7A = 0;
    this->max_length = -1.0;
    this->field_4 |= 0x100;
    this->field_7A = 0;
    this->field_104 = 0.0;
    this->field_100 = 0.0;
    this->field_128 = -1;
    this->field_DC = 0;
    this->field_E0 = 0;
    this->field_11C = 0;
    this->field_120 = 0;
    this->field_124 = 1.0;
    this->field_7B = 0;
    this->field_7C = nullptr;
    this->field_108 = 0;

    string_hash v5{"c_alpha"};
    this->set_material(v5);
    this->field_78 = 0;
    this->field_74 = nullptr;
    this->field_70 = nullptr;
    this->field_130 = nullptr;

    std::memset(this->field_15C, 0, sizeof(this->field_15C));

    this->field_8 = (this->field_8 & 0x8002041F) | 0xF;
    misc_render_objects.clear();
    this->field_142 = 0;
    this->field_150 = nullptr;
    this->field_154 = nullptr;
    this->field_158 = nullptr;
    this->field_140 = -1;
}

extern void PolytubeListAddNode(nglMesh *, nglBlendModeType, const math::VecClass<3, 1> &, Float,
                                const math::MatClass<4, 3> &, PCUV_ShaderMaterial *,
                                nglParamSet<nglShaderParamSet_Pool> *);
extern void TentacleListAddNode(nglMesh *, nglBlendModeType, const math::VecClass<3, 1> &, Float,
                                const math::MatClass<4, 3> &);

void polytube_misc_render_object::render(polytube *tube, Float dt)
{
    auto *ent = static_cast<entity *>(object.get_volatile_ptr());
    if (!tube || !ent)
        return;
    const vector3d point = tube->get_abs_po().slow_xform(tube->the_spline.calc_point_at_percent(percent));
    const auto &camera_po = g_game_ptr->get_current_view_camera(0)->get_abs_po();
    const vector3d forward = camera_po.get_z_facing();
    po placement;
    placement.set_po(-forward, camera_po.get_y_facing(), point - (tube->tube_radius + .01f) * forward);
    ent->get_rel_po() = placement;
    ent->dirty_family(false);
    if (ent->field_4 & 0x8004)
        ent->dirty_model_po_family();
    ent->po_changed();
    ent->set_render_color(color);
    ent->render(dt);
    ent->set_parent(nullptr);
}

void polytube::add_misc_render_object(polytube_misc_render_object *object)
{
    if (!misc_render_objects.contains(object))
        misc_render_objects.push_back(object);
}

bool polytube::remove_misc_render_object(polytube_misc_render_object *object)
{
    if (!misc_render_objects.contains(object))
        return false;
    misc_render_objects.common_erase(object, false);
    return true;
}

void polytube::_render(Float dt)
{
#if STANDALONE_SYSTEM
    static bool g_render_polytubes = true;
    if (!g_render_polytubes)
        return;
#else
    static Var<bool> g_render_polytubes{0x00922C64};
    if (!g_render_polytubes())
        return;
#endif
    field_150 = field_154 = field_158 = nullptr;
    std::fill(std::begin(field_15C), std::end(field_15C), 0);
    if (!field_78) {
        destroy_offsets();
        init_offsets();
    }
    const vector3d camera = get_abs_po().inverse_xform(vector3d(nglCurScene->ViewToWorld[3]));
    using ParamSet = nglParamSet<nglShaderParamSet_Pool>;
    ParamSet params{static_cast<ParamSet::nglParamSetType>(1)};
    if (field_140 < 0) {
        params = ParamSet{static_cast<ParamSet::nglParamSetType>(0)};
    } else {
        params.SetParam(nglTextureFrameParam{field_140});
        if (!field_142 && static_cast<uint32_t>(++field_140) == field_D0->m_texture->m_num_palettes)
            field_140 = 0;
    }
    if (field_11C.get_volatile_ptr() || field_120.get_volatile_ptr()) {
        auto *start = field_11C.get_volatile_ptr();
        auto *end = field_120.get_volatile_ptr();
        if (start && end)
            simulate_slack(start->get_abs_position(), end->get_abs_position(), field_124);
        else
            field_11C = field_120 = INVALID_HANDLE;
    }
    auto *points = &the_spline.control_pts;
    if (field_79) {
        rebuild_helper();
        points = &the_spline.curve_pts;
    }
    const auto scalar_property = [this](unsigned slot) {
        auto function = reinterpret_cast<float(__fastcall *)(polytube *, void *)>(get_vfunc(m_vtbl, slot));
        return function(this, nullptr);
    };
    float width = 1;
    if (field_E4) {
        width = scalar_property(0x214);
        scalar_property(0x224);
        scalar_property(0x21C);
        if (width < .01f)
            return;
    }
    const auto *end = points->end();
    unsigned count = 0;
    float distance = 0;
    for (const auto *point = points->begin(); point != end; ++point) {
        ++count;
        point = skip_coincident_points(point, end);
        if (max_length >= 0 && point + 1 != end) {
            distance += (point[1] - *point).length();
            if (distance > max_length) {
                ++count;
                break;
            }
        }
    }
    if (count < 2)
        return;
    const math::MatClass<4, 3> transform{get_abs_po().m};
    const color32 color = entity::get_render_color();
    const uint32_t packed_color = color32::to_int(color);
    auto blend = field_D0->m_blend_mode;
    if (color[3] != 255 || blend == 3 || blend == 2)
        blend = static_cast<nglBlendModeType>((field_7A != 0) + 2);
    const unsigned samples_per_control = count / the_spline.control_pts.size();
    const unsigned start_count = samples_per_control * field_E0;
    const unsigned end_count = samples_per_control * field_DC;
    const float percent_step = 1.0f / (count - 1);
    const auto write_curve = [&](auto &&write_point, auto &&write_clipped) {
        float percent = 0;
        float length = 0;
        unsigned index = 0;
        for (const auto *point = points->begin(); point != end; ++point, ++index, percent += percent_step) {
            write_point(*point, index, percent);
            point = skip_coincident_points(point, end);
            if (max_length >= 0 && point + 1 != end) {
                const float segment_length = (point[1] - *point).length();
                length += segment_length;
                if (length > max_length) {
                    const float ratio = 1.0f - (length - max_length) / segment_length;
                    write_clipped(*point + ratio * (point[1] - *point));
                    break;
                }
            }
        }
    };
    if (field_E4) {
        TentacleCustomIterator iter(count, field_E4, field_70, packed_color, tiles_per_meter, field_104);
        write_curve(
            [&](const vector3d &point, unsigned, float percent) {
                float radius, angle;
                Tentacle_ShaderMaterial::SampleTentacle(field_E8, percent, radius, angle);
                iter.write(point, radius * 4.5f * width);
            },
            [&](const vector3d &point) { iter.write(point, width); });
        close_vertices(iter.vertices.definition);
        field_150 = nglCloseMesh();
    } else {
        PolytubeCustomVertex::Iterator iter(
            count - end_count - start_count, field_D0, field_74, packed_color, tiles_per_meter, field_104);
        vector3d view;
        write_curve(
            [&](const vector3d &point, unsigned index, float) {
                view = point - camera;
                if (index >= start_count && index < count - end_count) {
                    iter.Write(point, view);
                    ++iter.field_0;
                }
            },
            [&](const vector3d &point) {
                iter.Write(point, view);
                ++iter.field_0;
            });
        if ((!start_count && !end_count) || count > start_count + end_count + 1) {
            close_vertices(iter.field_C.field_4);
            field_150 = nglCloseMesh();
        }
    }
    vector3d visual_center;
    auto center_function =
        reinterpret_cast<vector3d *(__fastcall *)(polytube *, void *, vector3d *)>(get_vfunc(m_vtbl, 0x2C));
    center_function(this, nullptr, &visual_center);
    const math::VecClass<3, 1> center{visual_center.x, visual_center.y, visual_center.z};
    const Float radius{scalar_property(0x28) + tube_radius};
    if (field_E4) {
        TentacleListAddNode(field_150, blend, center, radius, transform);
    } else {
        if ((!start_count && !end_count) || count > start_count + end_count + 1)
            PolytubeListAddNode(field_150, blend, center, radius, transform, field_D0, &params);
        const auto add_end_section =
            [&](unsigned section_count, bool at_start, PolytubeCustomMaterial *material, nglMesh *&mesh) {
                if (!section_count)
                    return;
                PolytubeCustomVertex::Iterator iter(
                    section_count + 1, material, field_74, packed_color, tiles_per_meter, field_104);
                unsigned index = 0;
                for (const auto *point = points->begin(); point != end; ++point, ++index) {
                    if (at_start ? index <= section_count : index >= count - section_count - 1) {
                        iter.Write(*point, *point - camera);
                        ++iter.field_0;
                    }
                    point = skip_coincident_points(point, end);
                }
                close_vertices(iter.field_C.field_4);
                mesh = nglCloseMesh();
                PolytubeListAddNode(mesh, blend, center, radius, transform, material, &params);
            };
        add_end_section(start_count, true, field_D4, field_154);
        add_end_section(end_count, false, field_D8, field_158);
    }
    for (auto *object : misc_render_objects)
        if (object->enabled && object->color[3] != 0)
            object->render(this, dt);
}

void polytube::set_control_pt(int index, const vector3d &a2)
{
    this->the_spline.set_control_pt(index, a2);
}

vector3d polytube::get_control_pt(int a3)
{
    auto &v4 = this->the_spline.get_control_pt(a3);

    auto a2 = this->get_abs_po().slow_xform(v4);
    return a2;
}

void polytube::rebuild_helper()
{
    if (this->the_spline.need_rebuild) {
        this->the_spline.rebuild_helper();
    }
}

void polytube::set_abs_control_pt(int index, const vector3d &a3)
{
    auto &abs_po = this->get_abs_po();

    auto v4 = abs_po.inverse_xform(a3);
    this->set_control_pt(index, v4);
}

void polytube::set_max_length(Float a2)
{
    this->max_length = a2;
}

void polytube::frame_advance_all_polytubes(Float elapsed)
{
    TRACE("polytube::frame_advance_all_polytubes");

    for (auto *current = active_polytubes(); current != nullptr;) {
        auto *next = current->field_68;
        const float scale = current->field_58 != nullptr ? static_cast<float>(current->field_58->sub_4ADE50())
                                                         : g_world_ptr->time_manager.field_0;
        if (current->m_vtbl != 0) {
            auto *address = get_vfunc(current->m_vtbl, 0x1A4);
            if (address != nullptr) {
                void(__fastcall * frame_advance)(polytube *, void *, Float) = CAST(frame_advance, address);
                frame_advance(current, nullptr, Float{scale * elapsed.value});
            }
        }
        current->field_150 = nullptr;
        current->field_154 = nullptr;
        current->field_158 = nullptr;
        for (auto &value : current->field_15C) {
            value = 0;
        }
        current->rebuild_helper();
        current->update_proximity_maps();
        current = next;
    }

    for (auto *current = inactive_polytubes(); current != nullptr; current = current->field_68) {
        current->field_150 = nullptr;
        current->field_154 = nullptr;
        current->field_158 = nullptr;
        for (auto &value : current->field_15C) {
            value = 0;
        }
        current->rebuild_helper();
        current->update_proximity_maps();
    }
}

void polytube::set_material(string_hash name)
{
    if (field_D0) {
        field_D0->destroy(0);
        mem_dealloc(field_D0, sizeof(*field_D0));
    }
    auto *texture = nglGetTexture(name.source_hash_code);
    field_D0 = new (mem_alloc(sizeof(PolytubeCustomMaterial)))
        PolytubeCustomMaterial(texture, static_cast<nglBlendModeType>(2), 72);
}

void polytube::set_material(PolytubeCustomMaterial *material)
{
    if (material) {
        if (field_D0)
            *field_D0 = *material;
        else
            field_D0 = new (mem_alloc(sizeof(*material))) PolytubeCustomMaterial(*material);
    } else {
        set_material(string_hash("c_alpha"));
    }
}

void polytube::set_material(Tentacle_ShaderMaterial *material)
{
    if (material) {
        if (field_E4)
            *field_E4 = *material;
        else
            field_E4 = new (mem_alloc(sizeof(*material))) Tentacle_ShaderMaterial(*material);
    }
}

void polytube::set_tiles_per_meter(Float a2)
{
    this->tiles_per_meter = a2;
    assert(tiles_per_meter > 0.0f);
}

void polytube::check_anims(bool a2)
{
    if (!this->pt_anims.empty() || a2) {
        int anim_size = this->pt_anims.size();
        auto pt_size = this->get_num_control_pts();
        assert(anim_size <= pt_size);

        while (anim_size < pt_size) {
            polytube_pt_anim pt_anim{};

            this->pt_anims.push_back(pt_anim);

            ++anim_size;

            assert(anim_size == static_cast<int>(pt_anims.size()));
        }

        assert(static_cast<int>(pt_anims.size()) == get_num_control_pts());
    }
}


void polytube::set_anim(int index, const vector3d &start, const vector3d &direction, float duration, unsigned flags)
{
    if (pt_anims.empty())
        pt_anims.reserve(the_spline.control_pts.size());
    check_anims(true);
    pt_anims[index].set_anim(start, direction, duration, flags);
    update_active_list();
}


void polytube::set_random_pt_anim(int index, float radius, float duration, unsigned flags)
{
    const vector3d direction{radius, 0.0f, 0.0f};
    set_anim(index, the_spline.control_pts[index], direction, duration, flags | 8);
}


void polytube::ifl_lock(int frame)
{
    field_140 = static_cast<int16_t>(frame);
    field_142 = 1;
}


void polytube::ifl_play()
{
    field_142 = 0;
}

void polytube::add_control_pt(const vector3d &a2)
{
    this->the_spline.add_control_pt(a2);
    this->check_anims(false);

    if (this->field_130) {
        this->destroy_tentacle_info();
        this->create_tentacle_info();
    }
}

void polytube::destroy_tentacle_info()
{
    if (field_130) {
        delete field_130;
        field_130 = nullptr;
        update_active_list();
    }
}

void polytube::create_tentacle_info()
{
    if (!field_130) {
        field_130 = new ai_tentacle_info(nullptr);
        field_130->create_tentacle(this);
        update_active_list();
    }
}

void polytube::reserve_control_pts(int num)
{
    this->the_spline.reserve_control_pts(num);
}

void polytube::set_force_start(bool a1)
{
    this->the_spline.set_force_start(a1);
}

PolytubeCustomVertex::Iterator::Iterator(unsigned count, PCUV_ShaderMaterial *material,
                                         PolytubeCustomOffset::Iterator *offsets, uint32_t color, float tiles,
                                         float phase)
    : field_0(0), field_4(count), field_8(offsets), field_40(color), field_44(tiles), field_48(phase)
{
    nglCreateMesh(NGLMESH_TEMP, 1, 0, nullptr);
    auto *definition = nglCreatePCUVVertexDef();
    nglVertexDef_MultipassMesh_Base::AddMeshSection(definition,
                                                    material ? reinterpret_cast<nglMaterialBase *>(&material->field_4)
                                                             : nullptr,
                                                    2 * offsets->field_8 * (count - 1),
                                                    count - 1,
                                                    0,
                                                    nullptr,
                                                    24,
                                                    D3DPT_TRIANGLESTRIP,
                                                    true);
    field_C = definition->CreateIterator();
}

void PolytubeCustomVertex::Iterator::Write(const vector3d &point, const vector3d &view)
{
    vector3d up = view;
    vector3d right;
    float phase = field_48;
    field_8->field_4 = 0;
    if (field_0 > 0) {
        vector3d direction = point - field_18;
        const float length = direction.length();
        direction = direction / length;
        if (field_0 == 1) {
            field_30 = vector3d::cross(direction, up);
            field_30.normalize();
            field_24 = vector3d::cross(field_30, direction);
            field_3C = field_48;
        }
        right = vector3d::cross(direction, up);
        right.normalize();
        up = vector3d::cross(right, direction);
        phase = field_3C - length * field_44;
        const float step = 1.0f / (field_8->field_8 - 1);
        float u = 0;
        field_C.BeginStrip(2 * field_8->field_8);
        for (; field_8->field_4 < static_cast<int>(field_8->field_8); ++field_8->field_4) {
            const auto &offset = field_8->field_0[field_8->field_4];
            const vector3d previous = field_18 + offset.x * field_30 + offset.y * field_24;
            const vector3d current = point + offset.x * right + offset.y * up;
            field_C.Write(previous, field_40, {u, field_3C});
            ++field_C;
            field_C.Write(current, field_40, {u, phase});
            ++field_C;
            u += step;
        }
    }
    field_18 = point;
    field_24 = up;
    field_30 = right;
    field_3C = phase;
}

void polytube_patch()
{
    {
        FUNC_ADDRESS(address, &polytube::_render);
        //set_vfunc(0x0088F46C, address);
    }

    REDIRECT(0x005584E8, polytube::frame_advance_all_polytubes);

    {
        FUNC_ADDRESS(address, &PolytubeCustomVertex::Iterator::Write);
        REDIRECT(0x005A60A6, address);
        REDIRECT(0x005A61E4, address);
        REDIRECT(0x005A6404, address);
        REDIRECT(0x005A658F, address);
    }
}
