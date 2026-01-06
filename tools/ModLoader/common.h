#pragma once
#define MOD_DIR "mods"
#include <map>

#if !defined(_DEBUG)
#define printf //
#endif

enum tlresource_type
{
    TLRESOURCE_TYPE_NONE = 0,
    TLRESOURCE_TYPE_TEXTURE = 1,
    TLRESOURCE_TYPE_MESH_FILE = 2,
    TLRESOURCE_TYPE_MESH = 3,
    TLRESOURCE_TYPE_MORPH_FILE = 4,
    TLRESOURCE_TYPE_MORPH = 5,
    TLRESOURCE_TYPE_MATERIAL_FILE = 6,
    TLRESOURCE_TYPE_MATERIAL = 7,
    TLRESOURCE_TYPE_ANIM_FILE = 8,
    TLRESOURCE_TYPE_ANIM = 9,
    TLRESOURCE_TYPE_SCENE_ANIM = 10,
    TLRESOURCE_TYPE_SKELETON = 11,
    TLRESOURCE_TYPE_Z = 12,
    TLRESOURCE_TYPE_IFL = 13,
    TLRESOURCE_TYPE_MPAL_TEXTURE = 14,
    TLRESOURCE_TYPE_MPAL_SUB_TEXTURE = 15
};

typedef int(__cdecl* cdecl_call)(...);

template<class To, class From>
constexpr
typename std::enable_if_t<sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<From>&&
    std::is_trivially_copyable_v<To>,
    To>
    // constexpr support needs compiler magic
    bit_cast(const From& src) noexcept {
    static_assert(
        std::is_trivially_constructible_v<To>,
        "This implementation additionally requires destination type to be trivially constructible");

    To dst;
    std::memcpy(&dst, &src, sizeof(To));
    return dst;
}

template<typename Func>
void REDIRECT(std::ptrdiff_t addr, Func my_func) {
    *bit_cast<uint8_t*>(addr) = 0xE8; //CALL
    *bit_cast<uint32_t*>(
        bit_cast<uint8_t*>(addr + 1)) = (bit_cast<uint32_t>(my_func)) - addr - 5;
    //sp_log("Patched address %08X with %s", addr, #my_func);
}

template<typename Func>
void SET_JUMP(std::ptrdiff_t addr, Func my_func) {
    *bit_cast<uint8_t*>(addr) = 0xE9; //JUMP

    *bit_cast<uint32_t*>(bit_cast<uint8_t*>(addr + 1)) = (bit_cast<uint32_t>(my_func)) - addr - 5;

    *bit_cast<uint8_t*>(addr + 0x5) = 0xC3; //RET
}
template<typename... Args>
decltype(auto) CDECL_CALL(int address, Args... args) {
    return (bit_cast<cdecl_call>(address))(args...);
}

struct resource_key
{
    uint32_t m_hash;
    uint32_t m_type;
};

struct string_hash {
    uint32_t source_hash_code;
};

struct resource_location
{
    resource_key field_0;
    unsigned __int8* field_8;
    int m_size;
};

struct mashable_vector__resource_location
{
    resource_location* m_data;
    unsigned __int16 m_size;
    char field_6;
    char empty;
};

struct fixedstring_4
{
    char field_0[32];
};

struct mashable_vector__fixedstring_4
{
    fixedstring_4* m_data;
    unsigned __int16 m_size;
    char field_6;
    char field_7;
};

struct resource_pack_group
{
    int field_0;
    int field_4;
    mashable_vector__fixedstring_4 field_8;
    int* field_10;
    int* field_14;
    int field_18;
    int field_1C;
};
struct mashable_vector__resource_pack_group
{
    resource_pack_group* m_data;
    unsigned __int16 m_size;
    bool field_6;
    char field_7;
};
struct resource_allocation_pool
{
    int field_0;
    int field_4;
    int field_8;
};

struct mashable_vector__resource_allocation_pool
{
    resource_allocation_pool* m_data;
    unsigned __int16 m_size;
    bool m_shared;
    char field_7;
};

struct tlresource_location
{
    string_hash name;
    unsigned int m_type;
    void* mesh_file;
};
struct mashable_vector__tlresource_location
{
    tlresource_location* m_data;
    unsigned __int16 m_size;
    char field_6;
    char field_7;
};

struct resource_pack_token
{
    void* field_0;
    int field_4;
};


struct resource_directory;

struct resource_pack_directory
{
    resource_directory* field_0;
    void* field_4;
    void* field_8;
    void* field_C;
    void* field_10;
    void* field_14;
    void* field_18;
    void* field_1C;
    void* field_20;
    int field_24;
    void* field_28;
    int field_2C;
    void* field_30;
    int field_34;
    void* field_38;
    int field_3C;
    void* field_40;
    int field_44;
    void* field_48;
    int field_4C;
    void* field_50;
    int field_54;
    void* field_58;
};

struct mashable_vector__ptr_resource_directory
{
    resource_directory** m_data;
    unsigned __int16 m_size;
    char field_6;
    char field_7;
};

struct resource_pack_slot
{
    void* m_vtbl;
    resource_key field_4;
    int m_slot_state;
    int slot_size;
    int pack_size;
    unsigned __int8* header_mem_addr;
    resource_pack_directory pack_directory;
    resource_pack_token field_78;
};


struct resource_directory
{
    mashable_vector__ptr_resource_directory parents;
    mashable_vector__resource_location resource_locations;
    mashable_vector__tlresource_location texture_locations;
    mashable_vector__tlresource_location mesh_file_locations;
    mashable_vector__tlresource_location mesh_locations;
    mashable_vector__tlresource_location morph_file_locations;
    mashable_vector__tlresource_location morph_locations;
    mashable_vector__tlresource_location material_file_locations;
    mashable_vector__tlresource_location material_locations;
    mashable_vector__tlresource_location anim_file_locations;
    mashable_vector__tlresource_location anim_locations;
    mashable_vector__tlresource_location scene_anim_locations;
    mashable_vector__tlresource_location skeleton_locations;
    mashable_vector__resource_pack_group field_68;
    mashable_vector__resource_allocation_pool field_70;
    resource_pack_slot* pack_slot;
    int base;
    unsigned __int8* field_80;
    unsigned int field_84;
    int field_88;
    int type_start_idxs[70];
    int type_end_idxs[70];
};


struct vm_thread
{
    char pad[0x20 + 0x184];
    char* _SP;
};



struct vm_stack {
    int field_0[96];
    char* buffer;
    char* SP;
    vm_thread* my_thread;
};

struct script_library_class
{
    struct function {
        enum entry_t {
            FIRST_ENTRY,
            RECALL_ENTRY,
        };

        struct {
            void(__fastcall* finalize)(function*, void* edx, bool);
            bool(__fastcall* __cl)(const function*, void* edx, vm_stack& stack, entry_t entry);
        } *m_vtbl;

#if SLC_NAME_FIELD
        const char* m_name;
#endif

        //0x0058EE30
        function(const char* name);

        //0x0058EDE0
        function(script_library_class* a2, const char* a3);

        //0x006794B0
        bool operator()(vm_stack& a2,
            script_library_class::function::entry_t a3) const;

        const char* get_name() const {
#if SLC_NAME_FIELD
            return this->m_name;
#else
            return "";
#endif
        }
    };


    int m_vtbl;
    char* name;
    int field_8;
    const char* field_C;
    script_library_class::function** funcs;
    int total_funcs;
    int next_func_slot;
    int flags;
};



struct std__vector__ptr_slc {
    int pad0;
    script_library_class** m_first;
    script_library_class** m_last;
    int padC;
};
inline std__vector__ptr_slc* SlcVec() {
    return reinterpret_cast<std__vector__ptr_slc*>(0x00965EC8);
}
inline size_t SlcCount() {
    auto* v = SlcVec();
    return size_t(v->m_last - v->m_first);
}
inline script_library_class* SlcAt(size_t i) {
    return SlcVec()->m_first[i];
}




const char* file_extensions[] =
{
  ".DDS",
  ".DDSMP",
  ".IFL",
  ".DESC",
  ".ENS",
  ".SPL",
  ".AB",
  ".QP",
  ".TRIG",
  ".PCSX",
  ".INST",
  ".FDF",
  ".PANEL",
  ".TXT",
  ".ICN",
  ".PCMESH",
  ".PCMORPH",
  ".PCMAT",
  ".COLL",
  ".PCPACK",
  ".PCSANIM",
  ".MSN",
  ".MARKER",
  ".HH",
  ".WAV",
  ".WBK",
  ".M2V",
  ".M2V",
  ".PFX",
  ".CSV",
  ".CLE",
  ".LIT",
  ".GRD",
  ".GLS",
  ".LOD",
  ".SIN",
  ".GV",
  ".SV",
  ".TOKENS",
  ".DSG",
  ".PATH",
  ".PTRL",
  ".LANG",
  ".SLF",
  ".VISEME",
  ".PCMESHDEF",
  ".PCMORPHDEF",
  ".PCMATDEF",
  ".MUT",
  ".ASG",
  ".BAI",
  ".CUT",
  ".INTERACT",
  ".CSV",
  ".CSV",
  "._ENTID_",
  "._ANIMID_",
  "._REGIONID_",
  "._AI_GENERIC_ID_",
  "._RADIOMSG_",
  "._GOAL_",
  "._IFC_ATTRIBUTE_",
  "._SIGNAL_",
  "._PACKGROUP_",
};

std::map <tlresource_type, int> tlres_to_res_key = {
    { TLRESOURCE_TYPE_NONE, -1 },
    { TLRESOURCE_TYPE_TEXTURE,  6 },
    { TLRESOURCE_TYPE_MESH_FILE, 21 },
    { TLRESOURCE_TYPE_MESH, 21 },

    { TLRESOURCE_TYPE_MORPH_FILE, 22 },
    { TLRESOURCE_TYPE_MORPH, 5 },

    { TLRESOURCE_TYPE_MATERIAL_FILE, 23 },
    { TLRESOURCE_TYPE_MATERIAL, 7 },

    { TLRESOURCE_TYPE_ANIM_FILE, 1 },
    { TLRESOURCE_TYPE_ANIM, 1 },

    { TLRESOURCE_TYPE_SCENE_ANIM, 26 },

    { TLRESOURCE_TYPE_SKELETON, 2 },
    { TLRESOURCE_TYPE_Z, 12 }, // ?
    { TLRESOURCE_TYPE_IFL, 6 },
    { TLRESOURCE_TYPE_MPAL_TEXTURE, 6 },
    { TLRESOURCE_TYPE_MPAL_SUB_TEXTURE, 6 },
};

const char* resource_key_type_ext[4][70] =
{
    // PS2
  {
    "none",
    ".PS2ANIM",
    ".PS2SKEL",
    ".ALS",
    ".ENT",
    ".ENTEXT",
    ".TM2",
    ".TM2MP",
    ".IFL",
    ".DESC",
    ".ENS",
    ".SPL",
    ".AB",
    ".QP",
    ".TRIG",
    ".PS2SX",
    ".INST",
    ".FDF",
    ".PANEL",
    ".TXT",
    ".ICN",
    ".PS2MESH",
    ".PS2MORPH",
    ".PS2MAT",
    ".COLL",
    ".PS2PACK",
    ".PS2SANIM",
    ".MSN",
    ".MARKER",
    ".HH",
    ".WAV",
    ".WBK",
    ".IPU",
    ".PSS",
    ".PFX",
    ".CSV",
    ".CLE",
    ".LIT",
    ".GRD",
    ".GLS",
    ".LOD",
    ".SIN",
    ".GV",
    ".SV",
    ".TOKENS",
    ".DSG",
    ".PATH",
    ".PTRL",
    ".LANG",
    ".SLF",
    ".VISEME",
    ".PS2MESHDEF",
    ".PS2MORPHDEF",
    ".PS2MATDEF",
    ".MUT",
    ".ASG",
    ".BAI",
    ".CUT",
    ".INTERACT",
    ".CSV",
    ".CSV",
    "._ENTID_",
    "._ANIMID_",
    "._REGIONID_",
    "._AI_GENERIC_ID_",
    "._RADIOMSG_",
    "._GOAL_",
    "._IFC_ATTRIBUTE_",
    "._SIGNAL_",
    "._PACKGROUP_"
  },
    // Xbox
  {
    "none",
    ".XBANIM",
    ".XBSKEL",
    ".ALS",
    ".ENT",
    ".ENTEXT",
    ".DDS",
    ".DDSMP",
    ".IFL",
    ".DESC",
    ".ENS",
    ".SPL",
    ".AB",
    ".QP",
    ".TRIG",
    ".XBSX",
    ".INST",
    ".FDF",
    ".PANEL",
    ".TXT",
    ".ICN",
    ".XBMESH",
    ".XBMORPH",
    ".XBMAT",
    ".COLL",
    ".XBPACK",
    ".XBSANIM",
    ".MSN",
    ".MARKER",
    ".HH",
    ".WAV",
    ".WBK",
    ".XMV",
    ".XMV",
    ".PFX",
    ".CSV",
    ".CLE",
    ".LIT",
    ".GRD",
    ".GLS",
    ".LOD",
    ".SIN",
    ".GV",
    ".SV",
    ".TOKENS",
    ".DSG",
    ".PATH",
    ".PTRL",
    ".LANG",
    ".SLF",
    ".VISEME",
    ".XBMESHDEF",
    ".XBMORPHDEF",
    ".XBMATDEF",
    ".MUT",
    ".ASG",
    ".BAI",
    ".CUT",
    ".INTERACT",
    ".CSV",
    ".CSV",
    "._ENTID_",
    "._ANIMID_",
    "._REGIONID_",
    "._AI_GENERIC_ID_",
    "._RADIOMSG_",
    "._GOAL_",
    "._IFC_ATTRIBUTE_",
    "._SIGNAL_",
    "._PACKGROUP_"
  },
    // GC
   {
     "none",
     ".GCANIM",
     ".GCSKEL",
     ".ALS",
     ".ENT",
     ".ENTEXT",
     ".GCT",
     ".GCTMP",
     ".IFL",
     ".DESC",
     ".ENS",
     ".SPL",
     ".AB",
     ".QP",
     ".TRIG",
     ".GCSX",
     ".INST",
     ".FDF",
     ".PANEL",
     ".TXT",
     ".TPL",
     ".GCMESH",
     ".GCMORPH",
     ".GCMAT",
     ".COLB",
     ".GCPACK",
     ".GCSANIM",
     ".MSN",
     ".MARKER",
     ".HH",
     ".WAV",
     ".WBK",
     ".H4M",
     ".H4M",
     ".PFX",
     ".CSV",
     ".CLE",
     ".LIT",
     ".GRD",
     ".GLS",
     ".LOD",
     ".SIN",
     ".GV",
     ".SV",
     ".TOKENS",
     ".DSG",
     ".PATH",
     ".PTRL",
     ".LANG",
     ".SLF",
     ".VISEME",
     ".GCMESHDEF",
     ".GCMORPHDEF",
     ".GCMATDEF",
     ".MUT",
     ".ASG",
     ".BAI",
     ".CUT",
     ".INTERACT",
     ".CSV",
     ".CSV",
     "._ENTID_",
     "._ANIMID_",
     "._REGIONID_",
     "._AI_GENERIC_ID_",
     "._RADIOMSG_",
     "._GOAL_",
     "._IFC_ATTRIBUTE_",
     "._SIGNAL_",
     "._PACKGROUP_"
   },
    // PC
    {
      "none",
      /*1*/".PCANIM",           // tlres = 8, 9
      /*2*/".PCSKEL",           // tlres = 11
      ".ALS",
      ".ENT",
      ".ENTEXT",
      /*6*/".DDS",              // tlres = 1
      ".DDSMP",
      ".IFL",
      ".DESC",
      ".ENS",
      ".SPL",
      ".AB",
      ".QP",
      ".TRIG",
      ".PCSX",
      ".INST",
      ".FDF",
      ".PANEL",
      ".TXT",
      ".ICN",
      /*21*/".PCMESH",            // tlres = 3
      ".PCMORPH",                   // tlres = 4
      ".PCMAT",                     // tlres = 6
      ".COLL",
      ".PCPACK",
      ".PCSANIM",                   // tlres = 10
      ".MSN",
      ".MARKER",
      ".HH",
      ".WAV",
      ".WBK",
      ".M2V",
      ".M2V",
      ".PFX",
      ".CSV",
      ".CLE",
      ".LIT",
      ".GRD",
      ".GLS",
      ".LOD",
      ".SIN",
      ".GV",
      ".SV",
      ".TOKENS",
      ".DSG",
      ".PATH",
      ".PTRL",
      ".LANG",
      ".SLF",
      ".VISEME",
      ".PCMESHDEF",             // tlres = 2
      ".PCMORPHDEF",            // tlres = 5
      ".PCMATDEF",              // tlres = 7
      ".MUT",
      ".ASG",
      ".BAI",
      ".CUT",
      ".INTERACT",
      ".CSV",
      ".CSV",
      "._ENTID_",
      "._ANIMID_",
      "._REGIONID_",
      "._AI_GENERIC_ID_",
      "._RADIOMSG_",
      "._GOAL_",
      "._IFC_ATTRIBUTE_",
      "._SIGNAL_",
      "._PACKGROUP_"
    }
};
