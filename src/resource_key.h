#pragma once

#include "mash.h"
#include "mstring.h"
#include "string_hash.h"

static inline constexpr auto EXTENSION_LENGTH = 16u;

enum resource_key_type {
    RESOURCE_KEY_TYPE_NONE = 0,
    RESOURCE_KEY_TYPE_ANIMATION = 1,
    RESOURCE_KEY_TYPE_NAL_SKL = 2,
    RESOURCE_KEY_TYPE_ALS_FILE = 3,
    RESOURCE_KEY_TYPE_ENTITY = 4,
    RESOURCE_KEY_TYPE_EXTERNAL_ENT = 5,
    RESOURCE_KEY_TYPE_TEXTURE = 6,
    RESOURCE_KEY_TYPE_MPAL_TEXTURE = 7,
    RESOURCE_KEY_TYPE_IFL = 8,
    RESOURCE_KEY_TYPE_DESCRIPTOR = 9,
    RESOURCE_KEY_TYPE_SCN_ENTITY = 10,
    RESOURCE_KEY_TYPE_SCN_AI_SPLINE_PATH = 11,
    RESOURCE_KEY_TYPE_SCN_AUDIO_BOX = 12,
    RESOURCE_KEY_TYPE_SCN_QUAD_PATH = 13,
    RESOURCE_KEY_TYPE_SCN_BOX_TRIGGER = 14,
    RESOURCE_KEY_TYPE_SCRIPT = 15,
    RESOURCE_KEY_TYPE_SCRIPT_INST = 16,
    RESOURCE_KEY_TYPE_NGL_FONT = 17,
    RESOURCE_KEY_TYPE_PANEL = 18,
    RESOURCE_KEY_TYPE_TEXTFILE = 19,
    RESOURCE_KEY_TYPE_ICON = 20,
    RESOURCE_KEY_TYPE_MESH = 21,
    RESOURCE_KEY_TYPE_MORPH = 22,
    RESOURCE_KEY_TYPE_MATERIAL = 23,
    RESOURCE_KEY_TYPE_COLLISION_MESH = 24,
    RESOURCE_KEY_TYPE_PACK = 25,
    RESOURCE_KEY_TYPE_SCENE_ANIM = 26,
    RESOURCE_KEY_TYPE_MISSION_TABLE = 27,

    RESOURCE_KEY_TYPE_SCRIPT_HEADER_FILE = 29,

    RESOURCE_KEY_TYPE_LOD = 40,
    RESOURCE_KEY_TYPE_SIN = 41,
    RESOURCE_KEY_TYPE_SCRIPT_GV = 42,
    RESOURCE_KEY_TYPE_SCRIPT_SV = 43,
    RESOURCE_KEY_TYPE_TOKEN_LIST = 44,
    RESOURCE_KEY_TYPE_DISTRICT_GRAPH = 45,
    RESOURCE_KEY_TYPE_PATH = 46,
    RESOURCE_KEY_TYPE_PATROL_DEF = 47,
    RESOURCE_KEY_TYPE_LANGUAGE = 48,
    RESOURCE_KEY_TYPE_SLF_LIST = 49,

    RESOURCE_KEY_TYPE_MESH_FILE_STRUCT = 51,
    RESOURCE_KEY_TYPE_MORPH_FILE_STRUCT = 52,
    RESOURCE_KEY_TYPE_MATERIAL_FILE_STRUCT = 53,

#ifdef TARGET_XBOX
    RESOURCE_KEY_TYPE_MASH_UNIT_TEST = 54,
#endif


    RESOURCE_KEY_TYPE_FX_CACHE,
    RESOURCE_KEY_TYPE_AI_STATE_GRAPH,
    RESOURCE_KEY_TYPE_BASE_AI,
    RESOURCE_KEY_TYPE_CUT_SCENE,
    RESOURCE_KEY_TYPE_AI_INTERACTION,
    RESOURCE_KEY_TYPE_GAB_DATABASE,
    RESOURCE_KEY_TYPE_SOUND_ALIAS_DATABASE,

    RESOURCE_KEY_TYPE_IFC_ATTRIBUTE = 67,

    RESOURCE_KEY_TYPE_Z = 70,
};

struct from_mash_in_place_constructor;
struct mash_info_struct;

struct resource_key {
    string_hash m_hash;
    resource_key_type m_type;

    resource_key() = default;

    resource_key(from_mash_in_place_constructor *a2);

    resource_key(string_hash hash, resource_key_type type) : m_hash(hash), m_type(type) {}

    resource_key(const resource_key &arg) : m_hash(arg.m_hash), m_type(arg.m_type) {}

    void initialize(mash::allocation_scope a2);

    bool operator==(const resource_key &key) const
    {
        return (this->m_hash == key.m_hash && this->m_type == key.m_type);
    }

    bool operator!=(const resource_key &key) const
    {
        return !(*this == key);
    }

    void set_type(resource_key_type type)
    {
        m_type = type;
    }

    decltype(auto) get_type() const
    {
        return m_type;
    }

    void operator=(const resource_key &a2);

    bool operator>=(const resource_key &a2) const
    {
        return this->m_hash > a2.m_hash || (this->m_hash == a2.m_hash && this->m_type > a2.m_type);
    }

    bool operator<=(const resource_key &a2) const
    {
        return this->m_hash < a2.m_hash || (this->m_hash == a2.m_hash && this->m_type < a2.m_type);
    }

    void unmash(mash_info_struct *, void *);

    mString get_platform_string(int a1) const;

    //0x004201C0
    void set(const resource_key &a2);

    bool is_set() const;

    //0x004201E0
    void set(string_hash a2, resource_key_type a3);

    //0x0041FFE0
    void destruct_mashed_class();

    //0x004200D0
    static void calc_resource_string_and_type_from_path(const char *in_string, mString *out_string,
                                                        resource_key_type *type_override);


    static resource_key_type resolve_extension(const char *target_string, bool a2);
};

using resource_key_type_ext_t = const char *[4][70];
extern resource_key_type_ext_t &resource_key_type_ext;

using resource_key_type_str_t = const char *[70];
extern resource_key_type_str_t &resource_key_type_str;

extern const char *to_string(resource_key_type type);

//0x004217B0
extern resource_key create_resource_key_from_path(const char *in_string, resource_key_type a3);

extern void resource_key_patch();
