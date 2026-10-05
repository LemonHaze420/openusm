#include "sniper_camera.h"

#include "common.h"
#include "memory.h"
#include "geometry_manager.h"
#include <algorithm>
#include <array>
#include <functional>

VALIDATE_SIZE(sniper_camera, 0x1B8u);
VALIDATE_OFFSET(sniper_camera, field_1A8, 0x1A8);
VALIDATE_OFFSET(sniper_camera, field_1B4, 0x1B4);

#if STANDALONE_SYSTEM
namespace {
void __fastcall sniper_destroy(sniper_camera *self, void *, bool release)
{
    self->~sniper_camera();
    if (release) sniper_camera::operator delete(self);
}
int __fastcall sniper_flavor(sniper_camera *, void *) { return 23; }
bool __fastcall sniper_identity(sniper_camera *, void *) { return true; }
void __fastcall sniper_advance(sniper_camera *self, void *, Float dt) { self->frame_advance(dt); }
void __fastcall sniper_sync(sniper_camera *self, void *, camera *source) { self->sync(*source); }
void __fastcall sniper_zoom(sniper_camera *self, void *, float zoom) { self->set_zoom(zoom); }
void __fastcall sniper_zoom_out(sniper_camera *self, void *, float amount, float duration)
{
    self->zoom_to(self->field_1A8 - amount, duration);
}
void __fastcall sniper_zoom_in(sniper_camera *self, void *, float amount, float duration)
{
    self->zoom_to(self->field_1A8 + amount, duration);
}
void __fastcall sniper_zoom_to(sniper_camera *self, void *, float zoom, float duration) { self->zoom_to(zoom, duration); }
float __fastcall sniper_get_zoom(sniper_camera *self, void *) { return self->field_1A8; }
}
#endif

void *sniper_camera::native_vtable()
{
#if STANDALONE_SYSTEM
    static auto table = [] {
        std::array<void *, 192> result;
        std::copy_n(static_cast<void **>(game_camera::native_vtable()), result.size(), result.data());
        result[0] = reinterpret_cast<void *>(sniper_destroy);
        result[0x54 / 4] = reinterpret_cast<void *>(sniper_flavor);
        result[0x84 / 4] = reinterpret_cast<void *>(sniper_identity);
        result[0x1A4 / 4] = reinterpret_cast<void *>(sniper_advance);
        result[0x294 / 4] = reinterpret_cast<void *>(sniper_sync);
        result[0x2D0 / 4] = reinterpret_cast<void *>(sniper_zoom);
        result[0x2D4 / 4] = reinterpret_cast<void *>(sniper_zoom_out);
        result[0x2D8 / 4] = reinterpret_cast<void *>(sniper_zoom_in);
        result[0x2DC / 4] = reinterpret_cast<void *>(sniper_zoom_to);
        result[0x2E0 / 4] = reinterpret_cast<void *>(sniper_get_zoom);
        return result;
    }();
    return table.data();
#else
    return reinterpret_cast<void *>(0x0088C510);
#endif
}

sniper_camera::sniper_camera(const string_hash &a2, entity *a3) : game_camera(a2, a3)
{
#if STANDALONE_SYSTEM
    m_vtbl = reinterpret_cast<int>(native_vtable());
    field_1A8 = 1.0f;
#endif
    this->field_1A0 = 0;
    this->field_1A4 = 0;
    this->field_1B0 = 0;
    this->field_1B4 = 0;

    this->field_1AC = 1.0;
}

void *sniper_camera::operator new(size_t size)
{
#if STANDALONE_SYSTEM
    return mem_alloc(size);
#else
    return _aligned_malloc(size, 4);
#endif
}

void sniper_camera::operator delete(void *ptr)
{
#if STANDALONE_SYSTEM
    mem_dealloc(ptr, sizeof(sniper_camera));
#else
    _aligned_free(ptr);
#endif
}

void sniper_camera::sync(camera &source)
{
    if (!is_externally_controlled()) {
        game_camera::_sync(source);
        field_1A0 = field_1A4 = 0;
    }
}

void sniper_camera::frame_advance(Float dt)
{
    if (!std::equal_to<float>{}(field_1A8, field_1AC)) {
        if (field_1B4 <= 0.0f) {
            field_1A8 = field_1AC;
        } else {
            field_1B4 = std::max(0.0f, field_1B4 - dt);
            field_1A8 += dt * field_1B0;
            if ((field_1B0 < 0.0f && field_1A8 <= field_1AC)
                || (field_1B0 > 0.0f && field_1A8 >= field_1AC)) {
                field_1A8 = field_1AC;
                field_1B4 = 0.0f;
            }
        }
    }
    geometry_manager::set_zoom(field_1A8);
    camera::_frame_advance(dt);
}

void sniper_camera::set_zoom(float zoom)
{
    field_1B4 = 0.0f;
    field_1AC = field_1A8 = zoom;
    geometry_manager::set_zoom(zoom);
}

void sniper_camera::zoom_to(float zoom, float duration)
{
    if (duration <= 0.0f) {
        set_zoom(zoom);
    } else {
        field_1AC = zoom;
        field_1B4 = duration;
        field_1B0 = (zoom - field_1A8) / duration;
    }
}
