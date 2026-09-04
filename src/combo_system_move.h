#pragma once

#include <cstdint>

#include "mash_virtual_base.h"
#include "mstring.h"
#include "mvector.h"
#include "string_hash.h"

struct combo_system_move : mash_virtual_base {
    struct trigger_info : mash_virtual_base {
        int field_4;
        int field_8;

        //0x00446AA0
        trigger_info();

        void initialize(bool a2);

        //virtual
        void _unmash(mash_info_struct *, void *);

        //virtual
        int _get_virtual_type_enum() const
        {
            return 101;
        }

        int _get_mash_sizeof() const
        {
            return sizeof(*this);
        }

        //virtual
        int get_mash_sizeof() const;

        static inline void *g_vtbl[]{nullptr,
                                     func_address(&_unmash),
                                     nullptr,
                                     func_address(&_get_virtual_type_enum),
                                     nullptr,
                                     func_address(&_is_or_is_subclass_of),
                                     func_address(&_get_mash_sizeof)};
    };

    struct target_info : mash_virtual_base {
        int field_4;
        int field_8;

        target_info();

        void initialize(bool a2);

        //virtual
        void _unmash(mash_info_struct *, void *);

        //virtual
        int _get_virtual_type_enum() const
        {
            return 100;
        }

        int _get_mash_sizeof() const
        {
            return sizeof(*this);
        }

        //virtual
        int get_mash_sizeof() const;

        static inline void *g_vtbl[]{nullptr,
                                     func_address(&_unmash),
                                     nullptr,
                                     func_address(&_get_virtual_type_enum),
                                     nullptr,
                                     func_address(&_is_or_is_subclass_of),
                                     func_address(&_get_mash_sizeof)};
    };

    struct link_info : mash_virtual_base {
        string_hash field_4;
        int field_8;
        int field_C;
        int field_10;

        link_info();

        //virtual
        void _unmash(mash_info_struct *a1, void *a3);

        //virtual
        int _get_virtual_type_enum() const
        {
            return 96;
        }

        int _get_mash_sizeof() const
        {
            return sizeof(*this);
        }

        //virtual
        int get_mash_sizeof() const;

        static inline void *g_vtbl[]{nullptr,
                                     func_address(&_unmash),
                                     nullptr,
                                     func_address(&_get_virtual_type_enum),
                                     nullptr,
                                     func_address(&_is_or_is_subclass_of),
                                     func_address(&_get_mash_sizeof)};
    };

    struct range_info : mash_virtual_base {
        int field_4;
        int field_8;
        int field_C;
        int field_10;

        range_info();

        void initialize(bool a2);

        //virtual
        void _unmash(mash_info_struct *a1, void *a3);

        //virtual
        int _get_virtual_type_enum() const
        {
            return 97;
        }

        int _get_mash_sizeof() const
        {
            return sizeof(*this);
        }

        //virtual
        int get_mash_sizeof() const;

        static inline void *g_vtbl[]{nullptr,
                                     func_address(&_unmash),
                                     nullptr,
                                     func_address(&_get_virtual_type_enum),
                                     nullptr,
                                     func_address(&_is_or_is_subclass_of),
                                     func_address(&_get_mash_sizeof)};
    };

    struct requirements : mash_virtual_base {
        trigger_info field_4;
        target_info field_10;
        range_info field_1C;
        mVector<link_info> field_30;
        void *field_44;

        requirements();

        void initialize(bool a2);

        //0x00481470
        //virtual
        void _unmash(mash_info_struct *a1, void *a3);

        //virtual
        int _get_virtual_type_enum() const
        {
            return 98;
        }

        int _get_mash_sizeof() const
        {
            return sizeof(*this);
        }

        //virtual
        int get_mash_sizeof() const;

        static inline void *g_vtbl[]{nullptr,
                                     func_address(&_unmash),
                                     nullptr,
                                     func_address(&_get_virtual_type_enum),
                                     nullptr,
                                     func_address(&_is_or_is_subclass_of),
                                     func_address(&_get_mash_sizeof)};
    };

    struct dialation_info : mash_virtual_base {
        int field_4;
        int field_8;
        int field_C;
        int field_10;

        dialation_info();

        //virtual
        void _unmash(mash_info_struct *, void *);

        //virtual
        int _get_virtual_type_enum() const
        {
            return 95;
        }

        int _get_mash_sizeof() const
        {
            return sizeof(*this);
        }

        //virtual
        int get_mash_sizeof() const;

        static inline void *g_vtbl[]{nullptr,
                                     func_address(&_unmash),
                                     nullptr,
                                     func_address(&_get_virtual_type_enum),
                                     nullptr,
                                     func_address(&_is_or_is_subclass_of),
                                     func_address(&_get_mash_sizeof)};
    };

    struct results : mash_virtual_base {
        string_hash field_4;
        string_hash field_8;
        string_hash field_C;
        mString field_10;
        int field_20;
        int field_24;
        int field_28;
        int field_2C;
        int field_30;
        int field_34;
        int field_38;
        int field_3C;
        dialation_info field_40;
        dialation_info field_54;
        bool field_68;
        int field_6C;
        int field_70;
        int field_74;
        int field_78;

        results();

        //0x0048CBB0
        results(const results &a2);

        void initialize(bool a2);

        //virtual
        ~results();

        //0x00471B30
        //virtual
        void _unmash(mash_info_struct *, void *);

        //virtual
        int _get_virtual_type_enum() const
        {
            return 99;
        }

        int _get_mash_sizeof() const
        {
            return sizeof(*this);
        }

        //virtual
        int get_mash_sizeof() const;

        static inline void *g_vtbl[]{nullptr,
                                     func_address(&_unmash),
                                     nullptr,
                                     func_address(&_get_virtual_type_enum),
                                     nullptr,
                                     func_address(&_is_or_is_subclass_of),
                                     func_address(&_get_mash_sizeof)};
    };

    results field_4;
    requirements field_80;

    //0x0043EDF0
    combo_system_move();

    //0x00471BA0
    //virtual
    void _unmash(mash_info_struct *a2, void *a3);

    //virtual
    int _get_virtual_type_enum() const
    {
        return 94;
    }

    int _get_mash_sizeof() const;

    //virtual
    int get_mash_sizeof() const;

    static inline void *g_vtbl[]{nullptr,
                                 func_address(&_unmash),
                                 nullptr,
                                 func_address(&_get_virtual_type_enum),
                                 nullptr,
                                 func_address(&_is_or_is_subclass_of),
                                 func_address(&_get_mash_sizeof)};
};

extern void combo_system_move_patch();
