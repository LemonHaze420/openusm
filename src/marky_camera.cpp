#include "marky_camera.h"

#include "common.h"
#include "oldmath_po.h"
#include "string_hash.h"
#include "memory.h"
#include "collide.h"
#include "local_collision.h"
#include <algorithm>
#include <array>
#include <cmath>

VALIDATE_SIZE(marky_camera, 0x1E0u);
VALIDATE_OFFSET(marky_camera, field_1B8, 0x1B8);
VALIDATE_OFFSET(marky_camera, field_1CC, 0x1CC);

#if STANDALONE_SYSTEM
namespace {
void __fastcall marky_destroy(marky_camera *self, void *, bool release)
{
    self->~marky_camera();
    if (release)
        marky_camera::operator delete(self);
}
int __fastcall marky_flavor(marky_camera *, void *)
{
    return 22;
}
bool __fastcall marky_identity(marky_camera *, void *)
{
    return true;
}
void __fastcall marky_advance(marky_camera *self, void *, Float dt)
{
    self->frame_advance(dt);
}
void __fastcall marky_sync(marky_camera *self, void *, camera *source)
{
    self->sync(*source);
}
void __fastcall marky_target(marky_camera *self, void *, const vector3d *target)
{
    self->camera_set_target(*target);
}
vector3d *__fastcall marky_get_target(marky_camera *self, void *, vector3d *out)
{
    *out = self->camera_get_target();
    return out;
}
void __fastcall marky_roll(marky_camera *self, void *, float value)
{
    self->camera_set_roll(value);
}
void __fastcall marky_collide(marky_camera *self, void *, bool value)
{
    self->camera_set_collide_with_world(value);
}
bool __fastcall marky_slide(marky_camera *self, void *, const vector3d *pos, const vector3d *target, float roll,
                            float speed)
{
    return self->camera_slide_to(*pos, *target, roll, speed);
}
bool __fastcall marky_slide_orbit(marky_camera *self, void *, const vector3d *center, float radius, float azimuth,
                                  float elevation, float speed)
{
    return self->camera_slide_to_orbit(*center, radius, azimuth, elevation, speed);
}
void __fastcall marky_orbit(marky_camera *self, void *, const vector3d *center, float radius, float azimuth,
                            float elevation)
{
    self->camera_orbit(*center, radius, azimuth, elevation);
}
}  // namespace
#endif

void *marky_camera::native_vtable()
{
#if STANDALONE_SYSTEM
    static auto table = [] {
        std::array<void *, 192> result;
        std::copy_n(static_cast<void **>(game_camera::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(marky_destroy);
        result[0x54 / 4] = reinterpret_cast<void *>(marky_flavor);
        result[0x7C / 4] = reinterpret_cast<void *>(marky_identity);
        result[0x1A4 / 4] = reinterpret_cast<void *>(marky_advance);
        result[0x294 / 4] = reinterpret_cast<void *>(marky_sync);
        result[0x2D0 / 4] = reinterpret_cast<void *>(marky_target);
        result[0x2D4 / 4] = reinterpret_cast<void *>(marky_get_target);
        result[0x2D8 / 4] = reinterpret_cast<void *>(marky_roll);
        result[0x2DC / 4] = reinterpret_cast<void *>(marky_collide);
        result[0x2E0 / 4] = reinterpret_cast<void *>(marky_slide);
        result[0x2E4 / 4] = reinterpret_cast<void *>(marky_slide_orbit);
        result[0x2E8 / 4] = reinterpret_cast<void *>(marky_orbit);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x0088C220);
#endif
}

marky_camera::marky_camera(const string_hash &a2) : game_camera(a2, nullptr)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<int>(native_vtable());
#endif
#if STANDALONE_SYSTEM
    const float invalid = bit_cast<float>(0xFFFFFFFFu);
    this->field_1A0 = {invalid, invalid, invalid};
    this->field_1AC = {0.0f, 0.0f, 1.0f};
#else
    static Var<vector3d> stru_960E24{0x00960E24};
    static Var<vector3d> stru_9225D4{0x009225D4};

    this->field_1A0 = stru_960E24();
    this->field_1AC = stru_9225D4();
#endif
    this->field_1BC = 0;
    this->field_1BD = 0;
    this->field_1C0 = {};

    this->field_1B8 = 0;
    this->field_1DC = 0;
    this->field_1D8 = -1001.0;
}

void *marky_camera::operator new(size_t size)
{
#if STANDALONE_SYSTEM
    return mem_alloc(size);
#else
    return _aligned_malloc(size, 4);
#endif
}

void marky_camera::operator delete(void *ptr)
{
#if STANDALONE_SYSTEM
    mem_dealloc(ptr, sizeof(marky_camera));
#else
    _aligned_free(ptr);
#endif
}

void marky_camera::set_affixed_x_facing(bool a2)
{
    this->field_1BD = a2;
    if (a2) {
        auto &v3 = this->get_rel_po();
        this->field_1C0 = v3.get_x_facing();
    }
}

void marky_camera::sync(camera &a2)
{
    if (!this->is_externally_controlled()) {
#if STANDALONE_SYSTEM
        game_camera::_sync(a2);
#else
        game_camera::sync(a2);
#endif
    }
}

void marky_camera::camera_set_collide_with_world(bool a2)
{
    this->field_1BC = a2;
}

void marky_camera::camera_set_target(const vector3d &target)
{
    field_1AC = target;
}
vector3d marky_camera::camera_get_target() const
{
    return field_1AC;
}
void marky_camera::camera_set_roll(float roll)
{
    field_1B8 = roll;
}

void marky_camera::make_po()
{
    po transform = get_rel_po();
    if (field_1BD) {
        auto forward = (field_1AC - get_abs_position()).normalized();
        field_1C0.normalize();
        auto up = vector3d::cross(forward, field_1C0).normalized();
        forward = vector3d::cross(field_1C0, up).normalized();
        transform.set_po(field_1C0, up, forward, transform.get_position());
    } else {
        transform.set_facing(field_1AC);
        if (std::fabs(field_1B8) >= 0.001f) {
            po rotation;
            rotation.set_rotate_z(field_1B8);
            po composed;
            po::compose(composed, transform, rotation);
            transform = composed;
        }
    }
    set_abs_po(transform);
}

void marky_camera::frame_advance(Float dt)
{
    camera::_frame_advance(dt);
    if (is_externally_controlled())
        return;
    auto position = frame_advance_shake(get_abs_position(), dt);
    if (field_1BC) {
        vector3d target = field_12C ? field_1CC : (m_parent ? m_parent->get_abs_po().slow_xform(field_1AC) : field_1AC);
        auto direction = position - target;
        const float distance = direction.length();
        auto *reg = get_primary_region();
        if (distance > 0.00001f) {
            const auto end = target + direction * ((distance + 0.3f) / distance);
            vector3d hit, normal;
            if (find_intersection(target,
                                  end,
                                  *local_collision::entfilter_line_segment_camera_collision,
                                  *local_collision::obbfilter_lineseg_test,
                                  &hit,
                                  &normal,
                                  &reg,
                                  nullptr,
                                  nullptr,
                                  false)) {
                position = hit + (target - position) * (0.3f / distance);
            }
        }
        position = collide_with_world(this, position - target, 0.3f, target, reg);
        field_1CC = position;
        field_12C = true;
        if (m_parent)
            position = m_parent->get_abs_po().inverse_xform(position);
    }
    set_abs_position(position);
    make_po();
}

bool marky_camera::camera_slide_to(const vector3d &position, const vector3d &target, float roll, float speed)
{
    const float inverse = 1.0f / speed;
    const auto delta = position - get_rel_position();
    const float distance = delta.length();
    const float factor =
        std::fpclassify(distance) == FP_ZERO || distance * inverse >= 0.5f ? inverse : std::min(1.0f, 0.5f / distance);
    const auto next = get_rel_position() + delta * factor;
    set_abs_position(next);
    field_1AC += (target - field_1AC) * inverse;
    field_1B8 += (roll - field_1B8) * inverse;
    return (position - next).length() < 0.001f && (target - field_1AC).length() < LARGE_EPSILON &&
           std::fabs(roll - field_1B8) < LARGE_EPSILON;
}

bool marky_camera::camera_slide_to_orbit(const vector3d &center, float radius, float azimuth, float elevation,
                                         float speed)
{
    auto position = m_parent ? center - m_parent->get_abs_position() : center;
    position += vector3d{std::cos(azimuth) * radius, std::sin(elevation) * radius, std::sin(azimuth) * radius};
    return camera_slide_to(position, center, field_1B8, speed);
}

void marky_camera::camera_orbit(const vector3d &center, float radius, float azimuth, float elevation)
{
    field_1AC = center;
    auto position = m_parent ? center - m_parent->get_abs_position() : center;
    position += vector3d{std::cos(azimuth) * radius, std::sin(elevation) * radius, std::sin(azimuth) * radius};
    set_abs_position(position);
}
