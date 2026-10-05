#include "poi.h"

#include "func_wrapper.h"
#include <cmath>

int &dword_938004 = var<int>(0x00938004);

vector3d point_of_interest::get_location() const
{
    entity *ent = nullptr;
    auto *v3 = &this->field_1C;
    if (this->field_1C.get_volatile_ptr() != nullptr && (ent = v3->get_volatile_ptr()) != nullptr) {
        return ent->get_abs_position();
    }

    return this->field_0;
}

namespace poi_manager {

point_of_interest **&poi_list = var<point_of_interest **>(0x0096CA1C);

int remove_point_of_interest(int index)
{
    if (index < 0 || index >= 75 || poi_list == nullptr || poi_list[index] == nullptr)
        return -1;

    delete poi_list[index];
    poi_list[index] = nullptr;
    if (index == dword_938004) {
        dword_938004 = -1;
        for (int i = 74; i >= 0; --i) {
            if (poi_list[i] != nullptr) {
                dword_938004 = i;
                break;
            }
        }
    }
    return index;
}

void cleanup()
{
    if (poi_list == nullptr)
        return;

    for (int i = 0; i < 75; ++i)
        remove_point_of_interest(i);

    operator delete(poi_list);
    poi_list = nullptr;
}

void check_init()
{
    if (poi_manager::poi_list == nullptr) {
        poi_list = (point_of_interest **)operator new(0x12Cu);
        for (int i = 0; i < 75; ++i) {
            poi_list[i] = nullptr;
        }

        dword_938004 = -1;
    }
}

int add_point_of_interest(const vector3d &position, int type, float radius, float duration,
                          vhandle_type<entity> owner)
{
    check_init();
    int available = -1;
    const bool attached = owner.get_volatile_ptr() != nullptr;
    for (int i = 0; i < 75; ++i) {
        auto *point = poi_list[i];
        if (point != nullptr && point->field_1C.field_0.get_goodies() != 0
            && point->field_1C.get_volatile_ptr() == nullptr) {
            remove_point_of_interest(i);
            point = nullptr;
        }
        if (point == nullptr) {
            if (available < 0)
                available = i;
            continue;
        }
        if (attached) {
            if (point->field_1C.field_0.get_goodies() == owner.field_0.get_goodies()
                && point->field_C == type)
                return -1;
        } else if (point->field_C == type && std::fabs(duration - point->field_10) < 0.0001f
                   && std::fabs(radius - point->field_18) < 0.0001f
                   && (point->field_0 - position).length2() < 0.1f) {
            return -1;
        }
    }
    if (available < 0)
        return -1;
    auto *point = new point_of_interest{};
    point->field_0 = position;
    point->field_C = type;
    point->field_10 = duration;
    point->field_14 = available;
    point->field_18 = radius;
    point->field_1C = owner;
    poi_list[available] = point;
    dword_938004 = available;
    for (int i = 74; i > available; --i)
        if (poi_list[i] != nullptr) {
            dword_938004 = i;
            break;
        }
    return available;
}

bool near_violence_poi(const vector3d &a1)
{
    poi_manager::check_init();
    if (poi_list == nullptr) {
        return false;
    }

    if (dword_938004 < 0) {
        return false;
    }

    for (int i = 0; i <= dword_938004; ++i) {
        auto *v3 = poi_list[i];
        if (v3 != nullptr) {
            auto v4 = v3->field_C;
            if (v4 == 1 || v4 == 3) {
                auto location = v3->get_location();
                auto v6 = location - a1;
                v6[0] = 0;

                auto v5 = v6.length2();

                auto v2 = [](point_of_interest *self) -> float {
                    return self->field_18 * self->field_18;
                }(v3);

                if (v2 >= v5) {
                    return true;
                }
            }
        }
    }

    return false;
}

}  // namespace poi_manager
