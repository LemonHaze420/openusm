#include "vm.h"

#include "common.h"
#include "debug_render.h"
#include "debug_string.h"
#include "fe_mission_text.h"
#include "femanager.h"
#include "femultilinetext.h"
#include "func_wrapper.h"
#include "game.h"
#include "glass_house_manager.h"
#include "igofrontend.h"
#include "localized_string_table.h"
#include "log.h"
#include "resource_key.h"
#include "resource_pack_location.h"
#include "resource_manager.h"
#include "script_lib.h"
#include "script_lib_debug_menu.h"
#include "script_manager.h"
#include "trace.h"
#include "utility.h"

#include <script_executable.h>
#include <script_object.h>
#include <vm_thread.h>

VALIDATE_SIZE(vm_stack, 0x18C);

mString generate_pack_name(const char *a2, vm_stack &stack)
{
    mString v6{stack.get_thread()->inst->get_parent()->get_parent()->field_0.to_string()};

    mString a1a{0, "%s_%s", a2, v6.c_str()};

    a1a.truncate(32);
    mString v4{};

    resource_key_type type = RESOURCE_KEY_TYPE_NONE;
    resource_key::calc_resource_string_and_type_from_path(a1a.c_str(), &v4, &type);

    return v4;
}

bool slf__set_mission_text__num__t::operator()(vm_stack &stack, script_library_class::function::entry_t)
{
#if 1

    auto v2 = stack.pop_num();
    const int v1 = v2 * 4.0f;
    float *v3 = (float *)(stack.get_SP() - v1);
    stack.pop(v1);

    auto *v4 = g_game_ptr->field_7C->lookup_scripttext_string((uint64_t)*v3);
    mString a2a{v4};

    sp_log("%s", a2a.c_str());

    mString result_string{};

    auto v5 = a2a.find("%", 0);
    auto v6 = (v5 == -1) - 2;
    if (v5 != -1) {
        auto v7 = v3 + 1;
        do {
            auto v8 = a2a.substr(v6 + 2, v5 - v6 - 2);
            sp_log("%s", v8.c_str());

            result_string += v8;
            sp_log("result_string: %s", result_string.c_str());

            switch (a2a.c_str()[v5 + 1]) {
            case '%':
                result_string.append("%");
                break;
            case 'd': {
                mString v9 = mString::from_int((uint64_t)*v7);

                result_string += v9;

                ++v7;
                break;
            }
            case 'f': {
                mString v10{*v7};

                result_string += v10;

                ++v7;
                break;
            }
            case 's': {
                auto v11 = g_game_ptr->field_7C->lookup_scripttext_string((uint64_t)*v7);
                result_string.append(v11);
                ++v7;
                break;
            }
            case 'z': {
                result_string.append(*bit_cast<const char **>(v7));
                ++v7;
                break;
            }
            default:
                break;
            }

            v6 = v5;
            if (v5 + 2 >= a2a.size()) {
                break;
            }

            v5 = a2a.find("%", v5 + 2);
        } while (v5 != -1);
    }

    if (v6 == -1) {
        result_string = a2a;

    } else if (v6 + 2 < a2a.size()) {
        auto v12 = a2a.substr(v6 + 2, 0xFFFFFFFF);

        result_string += v12;
    }

    mString v14{result_string.c_str()};

    result_string = FEMultiLineText::ReplaceEndlines(v14).c_str();

    sp_log("result_string: %s", result_string.c_str());

    mString temp_string{result_string.c_str()};

    g_femanager.IGO->field_20->set_text(*bit_cast<fe_mission_text::string *>(&temp_string));
    g_femanager.IGO->field_20->SetShown(1);

    SLF_DONE;
#else
    return (bool)THISCALL(0x00672C00, this, &stack, a2);
#endif
}

bool slf__bring_up_dialog_box_title__num__num__num__t::operator()(vm_stack &stack,
                                                                  script_library_class::function::entry_t a2)
{
    sp_log("script_library_class::function::entry_t: %d", a2);

    return (bool)THISCALL(0x00673240, this, &stack, a2);
}

bool slf__set_mission_text_box_flavor__num__t::operator()(vm_stack &a1, script_library_class::function::entry_t)
{
    auto num = a1.pop_num();

    sp_log("num = %d %f", num, num);

    g_femanager.IGO->field_20->set_flavor(num);
    return true;
}

bool __stdcall slf__get_pack_size__str__t__cl(vm_stack &stack, int)
{
    TRACE("slf__get_pack_size__str__t__cl");

    struct parms_t {
        using vm_str_t = const char *;

        vm_str_t str;
    };

    SLF_PARMS;

    auto out_key = create_resource_key_from_path(parms->str, RESOURCE_KEY_TYPE_PACK);
    resource_pack_location a2{};
    resource_manager::get_pack_file_stats(out_key, &a2, nullptr, nullptr);
    float result = a2.loc.m_size;
    if (a2.loc.m_size < 0) {
        result += 4.2949673e9;
    }

    SLF_RETURN;
    SLF_DONE;
}

void vm_patch()
{
    SET_JUMP(0x00676D70, slf__get_pack_size__str__t__cl);

    script_lib_debug_menu_patch();


    {
        FUNC_ADDRESS(address, &slf__add_3d_debug_str__vector3d__vector3d__num__str__t::operator());
        set_vfunc(0x0089A854, address);
    }

    {
        FUNC_ADDRESS(address, &slf__create_debug_menu_entry__str__str__t::operator());
        set_vfunc(0x0089C708, address);
    }

#if 0
    {
        FUNC_ADDRESS(address, &slf__bring_up_dialog_box_title__num__num__num__t::operator());
        set_vfunc(0x0089BC34, address);
    }

    {
        FUNC_ADDRESS(address, &slf__set_mission_text_box_flavor__num__t::operator());
        set_vfunc(0x0089BC68, address);
    }

    {
        FUNC_ADDRESS(address, &slf__is_point_inside_glass_house__vector3d__t::operator());
        set_vfunc(0x0089A544, address);
    }
#endif

    {
        FUNC_ADDRESS(address, &slf__set_mission_text__num__t::operator());
        //set_vfunc(0x0089BBFC, address);
    }
}
