#include "aps_native_curves.h"
#include "effect_mash_layout.h"

#include <array>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <new>
#include <utility>
#include <functional>

namespace {
using namespace aps_native;
CurveServices services{};
template <class T>
T *pointer(std::uint32_t word)
{
    return reinterpret_cast<T *>(static_cast<std::uintptr_t>(word));
}
template <class T>
T &at(void *p, unsigned offset)
{
    return *reinterpret_cast<T *>(static_cast<unsigned char *>(p) + offset);
}
template <class T>
const T &at(const void *p, unsigned offset)
{
    return *reinterpret_cast<const T *>(static_cast<const unsigned char *>(p) + offset);
}
[[noreturn]] void invalid_resource()
{
    std::abort();
}
double random_unit()
{
    return static_cast<double>(std::rand()) * 0.000030518509447574615f;
}
float clamp(float x, float lo, float hi)
{
    return x < lo ? lo : x > hi ? hi : x;
}
float dot(const float *a, const float *b)
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}
void normalize(float *v)
{
    const float length2 = dot(v, v);
    if (length2 > 0) {
        const float scale = 1.0f / std::sqrt(length2);
        for (int i = 0; i < 3; ++i)
            v[i] *= scale;
    }
}
void cross(float *out, const float *a, const float *b)
{
    out[0] = a[1] * b[2] - a[2] * b[1];
    out[1] = a[2] * b[0] - a[0] * b[2];
    out[2] = a[0] * b[1] - a[1] * b[0];
}
void transform(float *out, const float *v, const float *m, bool point)
{
    for (int i = 0; i < 3; ++i)
        out[i] = m[i] * v[0] + m[i + 4] * v[1] + m[i + 8] * v[2] + (point ? m[i + 12] : 0.0f);
}
void multiply_quaternion(float *out, const float *a, const float *b)
{
    out[0] = a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1];
    out[1] = a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0];
    out[2] = a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3];
    out[3] = a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2];
}
const float *parameters(const void *object)
{
    return pointer<const float>(static_cast<const Action *>(object)->parameters.data);
}
const void *domain(const void *object, unsigned index)
{
    return pointer<const void>(pointer<const std::uint32_t>(static_cast<const Action *>(object)->domains.data)[index]);
}
float *attribute(void *particle, const ParticleFormat &format, unsigned index)
{
    return &at<float>(particle, format.offsets[index]);
}
}

void aps_native_curve_set_services(const aps_native::CurveServices &value)
{
    services = value;
}

void aps_native_domain_sample(const void *object, int components, float *output)
{
    const unsigned type = aps_native_curve_type(object);
    const float *p = &at<float>(object, 4);
    switch (type) {
    case 8:
    case 9:
    case 13:
    case 14: {
        const unsigned width = (type == 8 || type == 13) ? 1 : 3;
        for (int i = 0; i < components; ++i) {
            double u = random_unit();
            if (type == 13 || type == 14)
                u = u < 0.5f ? 2 * u * (1 - u) : 2 * u * (u - 1) + 1;
            output[i] = p[i] + (p[i + width] - p[i]) * u;
        }
        return;
    }
    case 10:
    case 11:
    case 12:
        for (int i = 0; i < components; ++i)
            output[i] = p[i];
        return;
    case 15:
    case 16: {
        float x, y;
        if (type == 15) {
            for (int attempt = 0;; ++attempt) {
                x = (2 * random_unit() - 1) * p[9];
                y = (2 * random_unit() - 1) * p[9];
                if (x * x + y * y <= p[9] * p[9] || attempt == 99)
                    break;
            }
            for (int i = 0; i < 3; ++i)
                output[i] = p[i] + p[i + 3] * x + p[i + 6] * y;
        } else {
            const float angle = random_unit() * 6.2831854820251465f;
            x = std::cos(angle);
            y = std::sin(angle);
            for (int i = 0; i < 3; ++i)
                output[i] = p[i] + (p[i + 3] * x + p[i + 6] * y) * p[9];
        }
        return;
    }
    case 17:
    case 18:
    case 20:
    case 21:
    case 22:
        if (type == 18) {
            for (int i = 0; i < 3; ++i)
                output[i] = 2 * random_unit() - 1;
            normalize(output);
            for (int i = 0; i < 3; ++i)
                output[i] *= p[3];
        } else {
            for (int attempt = 0;; ++attempt) {
                for (int i = 0; i < 3; ++i)
                    output[i] = (type == static_cast<unsigned>(20 + i) ? random_unit() : 2 * random_unit() - 1) * p[3];
                if (dot(output, output) <= p[3] * p[3] || attempt == 99)
                    break;
            }
        }
        for (int i = 0; i < 3; ++i)
            output[i] += p[i];
        return;
    case 19: {
        const float u = random_unit();
        for (int i = 0; i < 3; ++i)
            output[i] = p[i] + p[i + 3] * u;
        return;
    }
    default:
        invalid_resource();
    }
}

bool aps_native_domain_contains(const void *object, int components, const float *value)
{
    const unsigned type = aps_native_curve_type(object);
    if (type < 8 || type > 22)
        invalid_resource();
    const float *p = &at<float>(object, 4);
    if (type == 15 || type == 16 || type == 19)
        return false;
    if (type == 17 || type == 18 || (type >= 20 && type <= 22)) {
        float delta[3];
        for (int i = 0; i < 3; ++i)
            delta[i] = value[i] - p[i];
        return (type < 20 || delta[type - 20] >= 0) && dot(delta, delta) <= p[3] * p[3];
    }
    const int width = (type == 8 || type == 10 || type == 13) ? 1 : type == 12 ? 4 : 3;

    for (int i = 0; i < components && i < width; ++i) {
        if (type == 10 || type == 11 || type == 12) {
            if (std::not_equal_to<float>{}(*value, p[i]))
                return false;
        } else if (!(*value >= p[i] && *value <= p[i + width]))
            return false;
    }
    return true;
}

int aps_native_emitter_count(const void *object, void *group, float dt)
{
    if (aps_native_curve_type(object) != 23)
        invalid_resource();
    const float *p = parameters(object);
    const double amount = static_cast<double>(dt) * p[2] + at<float>(group, 92);
    const int count = static_cast<int>(amount);
    at<float>(group, 92) = static_cast<float>(amount - count);
    return count * static_cast<int>(p[3]);
}

void aps_native_action_update(void *object, void *begin, void *end, void *group, float dt)
{
    const unsigned type = aps_native_curve_type(object);
    if (type < 23 || type > 54)
        invalid_resource();
    if (type == 23)
        return;
    const auto &format = at<ParticleFormat>(group, 120);
    const float *p = parameters(object);
    const float *matrix = &at<float>(group, 16);
    const bool local = at<std::uint8_t>(group, 161) != 0;
    float basis_right[3], basis_up[3], angle = 0;
    if (type >= 44 && type <= 47) {
        if (!services.camera_basis)
            invalid_resource();
        services.camera_basis(basis_right, basis_up, &angle);
    }
    if (type == 53) {
        const void *source = domain(object, 0);
        if (source) {
            float delta[3], scale;
            for (int i = 0; i < 3; ++i)
                delta[i] = at<float>(group, 80 + 4 * i) - at<float>(object, 48 + 4 * i);
            aps_native_domain_sample(source, 1, &scale);
            at<float>(group, 92) -= std::sqrt(dot(delta, delta)) * scale;
        }
        for (int i = 0; i < 3; ++i)
            at<float>(object, 48 + 4 * i) = at<float>(group, 80 + 4 * i);
        return;
    }
    float factor = 0, normal[3], plane_offset = 0, origin_base[3], direction[3], axis_inverse_length = 0;
    float movement[3], fixed_angle = 0;
    bool has_fixed_angle = false;
    if (type >= 28 && type <= 33) {
        if ((type & 1) == 0)
            factor = dt * p[2];
        else {
            const double x = std::log(static_cast<double>(p[2])) * dt;
            factor = static_cast<float>((0.5 * x + 1) * x + 1);
        }
    } else if (type >= 39 && type <= 41) {
        const double x = -static_cast<double>(dt) * p[2];
        factor = static_cast<float>(((static_cast<double>(0.1666666716337204f) * x + 0.5) * x + 1) * x + 1);
    } else if (type == 42) {
        for (int i = 0; i < 3; ++i)
            normal[i] = p[2 + i];
        normalize(normal);
        plane_offset = -dot(normal, p + 5);
    } else if (type >= 45 && type <= 47) {
        const float *axis = matrix + (type - 45) * 4;
        const float x = dot(basis_right, axis), y = dot(basis_up, axis);
        has_fixed_angle = std::not_equal_to<float>{}(x, 0.0f) || std::not_equal_to<float>{}(y, 0.0f);
        if (has_fixed_angle)
            fixed_angle = std::atan2(x, y) + angle;
    } else if (type >= 48 && type <= 50) {
        if (local)
            for (int i = 0; i < 3; ++i)
                origin_base[i] = p[2 + i];
        else
            transform(origin_base, p + 2, matrix, true);
        factor = dt * p[type == 48 ? 5 : 8];
        if (type != 48) {
            float endpoint[3];
            if (local)
                for (int i = 0; i < 3; ++i)
                    endpoint[i] = p[5 + i];
            else
                transform(endpoint, p + 5, matrix, true);
            for (int i = 0; i < 3; ++i)
                direction[i] = endpoint[i] - origin_base[i];
            const float length2 = dot(direction, direction);
            if (type == 50 || length2 > 0) {
                axis_inverse_length = 1 / std::sqrt(length2);
                for (int i = 0; i < 3; ++i)
                    direction[i] *= axis_inverse_length;
            }
        }
    } else if (type == 52 || type == 54) {
        for (int i = 0; i < 3; ++i)
            movement[i] = type == 52 ? at<float>(group, 80 + 4 * i) - at<float>(object, 48 + 4 * i)
                                     : at<float>(group, 64 + 4 * i) - at<float>(group, 80 + 4 * i);
    }
    auto *first = static_cast<unsigned char *>(begin);
    auto *last = static_cast<unsigned char *>(end);
    if (type == 54)
        first = last - format.stride * at<std::uint32_t>(group, 264);
    for (auto *particle = first; particle != last; particle += format.stride) {
        auto attr = [&](unsigned index) {
            return attribute(particle, format, index);
        };
        switch (type) {
        case 24: {
            float &age = *attr(9);
            age += dt / *attr(10);
            if (age >= 1) {
                age = 0.9998999834060669f;
                if (!services.kill)
                    invalid_resource();
                services.kill(group, particle);
            }
            break;
        }
        case 25: {
            const float age = *attr(9);
            *attr(4) = age < p[3] ? p[2] : age >= 1 ? 0 : (1 - age) * (p[2] / (1 - p[3]));
            break;
        }
        case 26: {
            const float age = *attr(9);
            *attr(4) = age < p[2]   ? age * (p[3] / p[2])
                       : age < p[4] ? p[3]
                       : age >= 1   ? 0
                                    : (1 - age) * (p[3] / (1 - p[4]));
            break;
        }
        case 27: {
            const float age = *attr(9), alpha = *attr(11);
            *attr(4) = age < p[2]   ? alpha * age * (1 / p[2])
                       : age < p[3] ? alpha
                       : age >= 1   ? 0
                                    : (1 - age) * alpha * (1 / (1 - p[3]));
            break;
        }
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33: {
            const unsigned index = type < 30 ? 1 : type < 32 ? 2 : 7;
            if ((type & 1) == 0)
                *attr(index) += factor;
            else
                *attr(index) *= factor;
            break;
        }
        case 34:
        case 35:
        case 36: {
            float *position = reinterpret_cast<float *>(particle), *velocity = attr(12);
            for (int i = 0; i < 3; ++i)
                position[i] += velocity[i] * dt;
            if (type == 35)
                *attr(6) += *attr(13) * dt;
            if (type == 36) {
                float *q = attr(5), *omega = attr(14);
                const float conjugate[4] = {-q[0], -q[1], -q[2], q[3]}, w[4] = {omega[0], omega[1], omega[2], 0};
                float temp[4], rotated[4], derivative[4];
                multiply_quaternion(temp, q, w);
                multiply_quaternion(rotated, temp, conjugate);
                rotated[3] = 0;
                multiply_quaternion(derivative, rotated, q);
                float length2 = 0;
                for (int i = 0; i < 4; ++i) {
                    q[i] += derivative[i] * (dt * 0.5f);
                    length2 += q[i] * q[i];
                }
                const float scale = 1 / std::sqrt(length2);
                for (int i = 0; i < 4; ++i)
                    q[i] *= scale;
            }
            break;
        }
        case 37:
            for (int i = 0; i < 3; ++i)
                attr(12)[i] += p[2 + i] * dt;
            break;
        case 38: {
            const float age = *attr(9);
            if (age >= p[2] && age <= p[6]) {
                const float t = (age - p[2]) / (p[6] - p[2]);
                for (int i = 0; i < 3; ++i)
                    attr(3)[i] = clamp(p[3 + i] + (p[7 + i] - p[3 + i]) * t, 0, 1);
            }
            break;
        }
        case 39:
        case 40:
        case 41: {
            float *v = attr(type == 39 ? 12 : type == 40 ? 13 : 14);
            for (int i = 0; i < (type == 40 ? 1 : 3); ++i)
                v[i] *= factor;
            break;
        }
        case 42: {
            float *position = attr(0), *velocity = attr(12);
            if (dot(normal, position) + plane_offset < 0) {
                const float speed = dot(normal, velocity);
                if (speed < 0)
                    for (int i = 0; i < 3; ++i)
                        velocity[i] -= normal[i] * ((p[8] + 1) * speed);
            }
            break;
        }
        case 43: {
            float frame = *attr(8) + dt * p[4];
            if (frame >= p[3])
                frame = frame - p[3] + p[2];
            if (frame < p[2])
                frame = p[3] - (p[2] - frame);
            *attr(8) = frame;
            break;
        }
        case 44: {
            float v[3];
            if (local)
                transform(v, attr(12), matrix, false);
            else
                for (int i = 0; i < 3; ++i)
                    v[i] = attr(12)[i];
            const float x = dot(basis_right, v), y = dot(basis_up, v);
            if (std::not_equal_to<float>{}(x, 0.0f) || std::not_equal_to<float>{}(y, 0.0f))
                *attr(6) = std::atan2(x, y) + angle;
            break;
        }
        case 45:
        case 46:
        case 47:
            if (has_fixed_angle)
                *attr(6) = fixed_angle;
            break;
        case 48:
        case 49:
        case 50: {
            float origin[3] = {origin_base[0], origin_base[1], origin_base[2]}, delta[3];
            float scale = factor;
            float *position = attr(0), *velocity = attr(12);
            if (type != 48) {
                for (int i = 0; i < 3; ++i)
                    delta[i] = position[i] - origin[i];
                const float projection = dot(delta, direction);
                if (type == 50)
                    scale *= 1 - clamp(projection * axis_inverse_length, 0, 1);
                for (int i = 0; i < 3; ++i)
                    origin[i] += direction[i] * projection;
            }
            for (int i = 0; i < 3; ++i)
                delta[i] = position[i] - origin[i];
            const float length2 = dot(delta, delta), bounded = length2 < 0.001f ? 0.001f : length2;
            const float inverse_length = 1 / std::sqrt(bounded), force = scale / bounded;
            for (int i = 0; i < 3; ++i)
                velocity[i] -= (delta[i] * inverse_length) * force;
            break;
        }
        case 51: {
            float *velocity = attr(12), *forward = attr(15), *side = attr(16), *up = attr(17), &turn = *attr(18),
                  &tilt = *attr(19);
            for (int i = 0; i < 3; ++i) {
                velocity[i] = forward[i] * p[2];
                forward[i] += (side[i] * turn) * dt;
                up[i] -= (side[i] * tilt) * dt;
            }
            normalize(forward);
            normalize(up);
            cross(side, forward, up);
            turn += (random_unit() * (p[4] - p[3]) + p[3]) * dt;
            tilt += (random_unit() * (p[8] - p[7]) + p[7]) * dt;
            turn = clamp(turn, p[5], p[6]);
            tilt = clamp(tilt, p[9], p[10]);
            break;
        }
        case 52:
        case 54: {
            if (const void *source = domain(object, 0)) {
                float scale;
                aps_native_domain_sample(source, 1, &scale);
                for (int i = 0; i < 3; ++i)
                    attr(type == 52 ? 12 : 0)[i] += movement[i] * scale;
            }
            break;
        }
        default:
            invalid_resource();
        }
    }
    if (type == 52)
        for (int i = 0; i < 3; ++i)
            at<float>(object, 48 + 4 * i) = at<float>(group, 80 + 4 * i);
}

void aps_native_actions_run(const void *actions, void *group, float time, float dt, bool reset, const float *modifiers)
{
    const auto &list = *static_cast<const aps_native::Container *>(actions);
    const auto *items = pointer<const std::uint32_t>(list.data);
    if (!items)
        return;
    if (dt > 0.3f)
        dt = 0.3f;
    unsigned index = 0;
    for (; index < list.count; ++index) {
        auto *action = pointer<aps_native::Action>(items[index]);
        if (action->is_operator)
            break;
        const float *window = parameters(action);
        if (!reset && window[0] <= time && time <= window[1]) {
            if (!services.emit)
                invalid_resource();
            services.emit(action, nullptr, nullptr, group, dt, modifiers);
        }
    }
    auto *begin = pointer<unsigned char>(at<std::uint32_t>(group, 240));
    const auto &format = at<aps_native::ParticleFormat>(group, 120);
    const auto count = at<std::uint32_t>(group, 244);
    if (!count)
        return;
    auto *end = begin + format.stride * count;
    for (; index < list.count; ++index) {
        auto *action = pointer<aps_native::Action>(items[index]);
        const float *window = parameters(action);
        if (window[0] <= time && time <= window[1])
            aps_native_action_update(action, begin, end, group, dt);
    }
}

namespace {
struct Vtable {
    std::uintptr_t slots[13];
};
void __fastcall release_entry(void *object, void *)
{
    aps_native_curve_release(object);
}
void __fastcall retail_empty(void *, void *) {}
void __fastcall unmash_entry(void *object, void *, void *info, int argument)
{
    if (aps_native_curve_type(object) >= 23) {
        if (!services.unmash)
            invalid_resource();
        services.unmash(object, info, argument);
    }
}
void *__fastcall delete_entry(void *object, void *, unsigned flags)
{
    aps_native_curve_release(object);
    if (flags & 1)
        ::operator delete(object);
    return object;
}
template <unsigned Type>
unsigned __fastcall type_entry(const void *, void *)
{
    return Type;
}
template <unsigned Type>
bool __fastcall subclass_entry(const void *, void *, unsigned type)
{
    return Type >= 48 && type == 56;
}
bool __fastcall is_type_entry(const void *object, void *, unsigned type)
{
    const unsigned actual = aps_native_curve_type(object);
    return type == actual || (actual >= 48 && type == 56);
}
void __fastcall sample_entry(const void *object, void *, int count, float *out)
{
    aps_native_domain_sample(object, count, out);
}
bool __fastcall contains_entry(const void *object, void *, int count, const float *v)
{
    return aps_native_domain_contains(object, count, v);
}
void __fastcall update_entry(void *object, void *, void *begin, void *end, void *group, float dt)
{
    aps_native_action_update(object, begin, end, group, dt);
}
void __fastcall emit_entry(void *object, void *, void *begin, void *end, void *group, float dt, const float *modifiers)
{
    if (aps_native_curve_type(object) == 23) {
        if (!services.emit)
            invalid_resource();
        services.emit(object, begin, end, group, dt, modifiers);
    }
}
int __fastcall count_entry(const void *object, void *, void *group, float dt)
{
    return aps_native_emitter_count(object, group, dt);
}
float __fastcall one_entry(const void *, void *)
{
    return 1.0f;
}
constexpr std::uint32_t names[] = {0x31644278, 0x33644278, 0x31645074, 0x33645074, 0x34645074, 0x31644275, 0x33644275,
                                   1147761507, 1131570028, 1399875685, 1399870325, 1281977957, 1481134960, 1497912176,
                                   1514689392, 1400005441, 1281975909, 1097614948, 1095125327, 1380337999, 1282298723,
                                   1165513571, 1280533335, 1163092823, 1280533320, 1163092808, 1349733750, 1299150437,
                                   1330474870, 1181708899, 1131172712, 1449935986, 1096172658, 1447122500, 1464881766,
                                   1431717490, 1097754452, 1096041816, 1096041817, 1096041818, 1346466930, 1279358066,
                                   1145848180, 1263821173, 1095320918, 1095320909, 1095320912};
template <unsigned Type>
std::uint32_t __fastcall name_entry(const void *, void *)
{
    return names[Type - 8];
}
template <unsigned Type>
unsigned __fastcall size_entry(const void *, void *)
{
    return effect_mash::aps_sizes[Type];
}
template <class T>
std::uintptr_t address(T fn)
{
    return reinterpret_cast<std::uintptr_t>(fn);
}
template <unsigned Type>
Vtable make_vtable()
{
    return {{address(release_entry),
             address(retail_empty),
             address(unmash_entry),
             address(delete_entry),
             address(type_entry<Type>),
             address(subclass_entry<Type>),
             address(is_type_entry),
             Type < 23 ? address(sample_entry) : address(update_entry),
             Type < 23 ? address(contains_entry) : address(emit_entry),
             address(name_entry<Type>),
             address(one_entry),
             address(size_entry<Type>),
             address(count_entry)}};
}
template <std::size_t... I>
std::array<Vtable, sizeof...(I)> make_tables(std::index_sequence<I...>)
{
    return {{make_vtable<I + 8>()...}};
}
const auto tables = make_tables(std::make_index_sequence<47>{});
bool inside(const Container &container, const void *value)
{
    const auto start = reinterpret_cast<std::uintptr_t>(&container), p = reinterpret_cast<std::uintptr_t>(value);
    return p >= start && p <= start + container.end_offset;
}
void clear_container(Container &container)
{
    void *data = pointer<void>(container.data);
    if (!inside(container, data))
        ::operator delete[](data);
    container = {};
}
}

void aps_native_curve_fixup(void *object, unsigned type)
{
    static_assert(sizeof(void *) == 4, "APS resources require the 32-bit PC ABI");
    if (type < 8 || type > 54)
        invalid_resource();
    at<std::uint32_t>(object, 0) = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&tables[type - 8]));
    if (type == 23)
        static_cast<Emitter *>(object)->field_30 = 0;
}
unsigned aps_native_curve_type(const void *object)
{
    const auto *vtable = pointer<const Vtable>(at<std::uint32_t>(object, 0));
    using GetType = unsigned(__fastcall *)(const void *, void *);
    return reinterpret_cast<GetType>(vtable->slots[4])(object, nullptr);
}
void aps_native_curve_release(void *object)
{
    if (aps_native_curve_type(object) < 23)
        return;
    auto &action = *static_cast<Action *>(object);
    clear_container(action.parameters);
    if (action.owns_domains && action.domains.data) {
        auto *items = pointer<std::uint32_t>(action.domains.data);
        for (unsigned i = 0; i < action.domains.count; ++i)
            if (void *item = pointer<void>(items[i])) {
                const bool mashed = inside(action.domains, item);
                aps_native_curve_release(item);
                if (!mashed)
                    ::operator delete(item);
                items[i] = 0;
            }
    }
    clear_container(action.domains);
}
