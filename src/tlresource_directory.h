#pragma once

#include "tlresourcedirectory.h"

#include "debugutil.h"
#include "os_developer_options.h"
#include "resource_directory.h"

template<typename, typename>
struct tlInstanceBankResourceDirectory;

template<typename T0, typename T1>
struct tlresource_directory : tlResourceDirectory<T0, T1> {
    resource_directory *field_4;

    tlresource_directory();

    void clear() {
        this->field_4 = nullptr;
    }

    void set_resource_directory(resource_directory *dir) {
        this->field_4 = dir;
    }

    //virtual
    void finalize(bool a1) {
        this->~tlresource_directory();

        if (a1) {
            tlMemFree(this);
        }
    }

    //virtual
    T0 * _Find(const T1 &a2)
    {
        TRACE((get_type_name<tlresource_directory<T0, T1>>() + "::Find").c_str());

        assert(tlres_type != TLRESOURCE_TYPE_NONE && "This type must have a matching tlresource type");

        T0 *v5 = nullptr;
        if (this->field_4 != nullptr)
        {
            v5 = CAST(v5, this->field_4->get_tlresource(a2, tlres_type));
        }

        if ( v5 == nullptr && system_dir != nullptr )
        {
            v5 = system_dir->Find(a2);
            bool SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString {"SHOW_RESOURCE_SPAM"});
            if ( v5 != nullptr )
            {
                if ( SHOW_RESOURCE_SPAM )
                {
                    auto *v2 = a2.c_str();
                    debug_print_va("found tlresource %s in system directory", v2);
                }
            }
            else if ( SHOW_RESOURCE_SPAM )
            {
                auto *v3 = a2.c_str();
                debug_print_va("didn't find tlresource %s in system directory", v3);
            }
        }

        if ( v5 == nullptr )
        {
            v5 = (T0 *) default_tlres;
        }

        return v5;
    }

    //virtual
    T0 * _Find(uint32_t a2) {
        T0 *v3 = nullptr;
        if ( this->field_4 != nullptr ) {
            v3 = (T0 *) this->field_4->get_tlresource(a2, tlres_type);
        }

        if ( v3 == nullptr && system_dir != nullptr )
        {
            auto SHOW_RESOURCE_SPAM = os_developer_options::instance->get_flag(mString{"SHOW_RESOURCE_SPAM"});

            v3 = system_dir->Find(a2);
            if ( v3 != nullptr )
            {
                if ( SHOW_RESOURCE_SPAM )
                {
                    debug_print_va("found tlresource %08x in system directory", a2);
                }
            }
            else if ( SHOW_RESOURCE_SPAM )
            {
                debug_print_va("didn't find tlresource %08x in system directory", a2);
            }
        }

        if ( v3 == nullptr ) {
            v3 = default_tlres;
        }

        return v3;
    }

    //virtual
    void _Add(T0 *) {}

    static tlInstanceBankResourceDirectory<T0, T1> *& system_dir;

    static T0 *& default_tlres;

    static tlresource_type tlres_type;
};

extern void tlresource_directory_patch();
