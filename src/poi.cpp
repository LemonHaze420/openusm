#include "poi.h"

#include "func_wrapper.h"

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

void cleanup()
{
    CDECL_CALL(0x006C4700);
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
