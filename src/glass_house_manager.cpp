#include "glass_house_manager.h"

#include "func_wrapper.h"
#include "glass_house.h"
#include "log.h"
#include "os_developer_options.h"
#include "utility.h"
#include "variables.h"

#if !STANDALONE_SYSTEM

bool & glass_house_manager::enabled = var<bool>(0x00921D78);

_std::vector<glass_house *>  (& glass_house_manager::glass_houses)[3] = var<_std::vector<glass_house *>[3]>(0x0096007C);

#else

bool & glass_house_manager::enabled = []() -> auto & {
    static bool g_enabled {};
    return g_enabled;
}();

_std::vector<glass_house *>  (& glass_house_manager::glass_houses)[3] = []() -> auto & {
    static _std::vector<glass_house *>  g_glass_houses[3] {};
    return g_glass_houses;
}();

#endif

bool glass_house_manager::is_enabled() {
    bool result = enabled && os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(122));

    return result;
}

bool glass_house_manager::is_point_in_glass_house(const vector3d &a1) {
    return (bool) CDECL_CALL(0x00538570, &a1);
}

void glass_house_manager::show_glass_houses()
{
    for ( auto i = 0; i < 3; ++i )
    {
        auto &array = glass_houses[i];
        for (auto &h : array)
        {
            h->render();
        }
    }
}

void glass_house_manager::remove_glass_house(string_hash a2)
{
    for ( int i = 0; i < 3; ++i )
    {
        auto it = glass_houses[i].begin();
        auto end = glass_houses[i].end();
        for ( ; it != end ; ++it)
        {
            if ( (*it)->get_name() == a2 )
            {
                glass_houses[i].erase(it);
                return;
            }
        }
    }

}

void glass_house_manager_patch() {
    REDIRECT(0x004645E6, glass_house_manager::is_enabled);
    REDIRECT(0x005BB673, glass_house_manager::is_enabled);
    REDIRECT(0x007421D2, glass_house_manager::is_enabled);
}
