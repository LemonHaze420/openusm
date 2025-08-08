#pragma once

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

std::vector<std::string> script_static_funcs;
std::map<std::string, std::vector<std::string>> script_class_funcs;

struct entity_base_vhandle
{
    unsigned int e_arg;
};
enum EntityExtFlags : unsigned __int32
{
    EXTFLAG_NONE = 0x0,
    EXTFLAG_DIRTY = 0x40,
    EXTFLAG_FORCE_UPDATE = 0x100,
    EXTFLAG_UPDATE_VIA_REGIONLINK = 0x2000,
    EXTFLAG_SCENE_OWNED = 0x8000,
    EXTFLAG_NO_DEALLOC_ABS_PO = 0x20000,
    EXTFLAG_NO_ZERO_VELOCITY = 0x40000,
    EXTFLAG_DYNAMIC_COLLISION_ACTOR = 0x80000,
    EXTFLAG_ALREADY_MOVED = 0x800000,
    EXTFLAG_PHYSICS_MOVED = 0x2000000,
    EXTFLAG_VISUAL_RADIUS_MINIMAL = 0x1000000,
    EXTFLAG_MODEL_REL_PO = 0x8000000,
    EXTFLAG_DIRTY_ABS_PO = 0x10000000,
    EXTFLAG_STATIC_ABS_PO = 0x40000000,
    EXTFLAG_ACTOR_DELUXE = 0x80000000,
    EXTFLAG_VISUAL_RADIUS_REDUCED = 0x20,
    EXTFLAG_PHYSICS_SUSPENDED = 0x4000,
    EXTFLAG_PARSED = 0x400,
};
enum EntityFlags : unsigned __int32
{
    FLAG_HAS_COLLISION = 0x2,
    FLAG_MODEL_DEPENDENT_PO = 0x4,
    FLAG_DISABLE_REGION_LINK = 0x8,
    FLAG_FADE_DISTANCE_MASK = 0xF,
    FLAG_HAS_CHARACTER_COLLISION = 0x10,
    FLAG_HAS_TERRAIN_COLLISION = 0x20,
    FLAG_DIRTY = 0x40,
    FLAG_CAN_WALK_ON = 0x80,
    FLAG_RENDERABLE = 0x100,
    FLAG_HAS_BOX_TRIGGER = 0x400,
    FLAG_SCALE_VISUAL_RADIUS = 0x800,
    FLAG_IS_VISIBLE = 0x200,
    FLAG_IS_ACTIVE = 0x2000,
    FLAG_COLLISION_ACTIVE = 0x4000,
    FLAG_CONGLOMERATE_MEMBER = 0x8000,
    FLAG_TRAFFIC_LIGHT_0 = 0x20000,
    FLAG_TRAFFIC_LIGHT_1 = 0x40000,
    FLAG_ACTIVE_DYNAMIC_ENTITY = 0x80000,
    FLAG_NO_COLLISION_GEOMETRY = 0x200000,
    FLAG_CAMERA_FROZEN = 0x400000,
    FLAG_IN_REGION_SYSTEM = 0x10000000,
    FLAG_HAS_COLLISION_GEO = 0x20000000,
};
struct matrix4x4 {
    float m[4][4];
};
typedef matrix4x4 po;

struct entity_base
{
    char* m_vtbl;
    EntityFlags flags;
    EntityExtFlags ext_flags;
    po* my_rel_po;
    string_hash field_10;
    po* my_abs_po;
    char* field_18;
    entity_base_vhandle my_handle;
    entity_base* m_parent;
    entity_base* m_child;
    entity_base* m_child_1;
    __int16 proximity_map_cell_reference_count;
    char m_timer;
    char field_2F;
    char* adopted_children;
    char* my_conglom_root;
    char* my_sound_and_pfx_interface;
    __int16 field_3C;
    __int16 field_3E;
    char field_40;
    char field_41;
    unsigned __int8 rel_po_idx;
    char proximity_map_reference_count;
};
enum EntityClassID : unsigned __int16
{
    ENTITY_BASE = 0x0,
    SIGNALLER = 0x1,
    ENTITY = 0x2,
    ACTOR = 0x3,
    BEAM = 0x4,
    CONGLOMERATE = 0x5,
    CONGLOMERATE_CLONE = 0x6,
    GRENADE = 0x7,
    ITEM = 0x8,
    HANDHELD_ITEM = 0x9,
    GUN = 0xA,
    MELEE_ITEM = 0xB,
    THROWN_ITEM = 0xC,
    LENSFLARE = 0xD,
    LIGHT_SOURCE = 0xE,
    MANIP_OBJ = 0xF,
    MARKER = 0x10,
    PARKING_MARKER = 0x11,
    WATER_EXIT_MARKER = 0x12,
    RECTANGLE_MARKER = 0x13,
    ANCHOR_MARKER = 0x14,
    LINE_ANCHOR = 0x15,
    NEOLIGHT = 0x16,
    PFX_ENTITY = 0x17,
    POLYTUBE = 0x18,
    SWITCH_OBJ = 0x19,
    VISUAL_ITEM = 0x1A,
    AI_COVER_MARKER = 0x1B,
};

static lua_State* g_L;

entity_base* __cdecl find_entity(const string_hash* a1, EntityClassID a2, bool a3)
{
    return (entity_base*)CDECL_CALL(0x004DC300, a1, a2, a3);
}


int lua_find_entity(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    string_hash hash = { to_hash(name) };
    entity_base* entity = find_entity(&hash, (EntityClassID)(SWITCH_OBJ | BEAM), 0);
    if (!entity)
        return luaL_error(L, "Entity '%s' not found", name);

    lua_pushlightuserdata(L, entity);
    return 1;
}


static void enumerate_scripts()
{
    fs::path scriptsDir = fs::current_path() / "scripts";
    if (!fs::is_directory(scriptsDir))
        return;

    for (const auto& entry : fs::directory_iterator(scriptsDir)) {
        if (!entry.is_regular_file())
            continue;

        const fs::path& path = entry.path();
        std::string ext = transformToLower(path.extension().string());
        if (ext != ".lua")
            continue;

        printf("[Lua] Loading %s...\n", path.string().c_str());
        if (luaL_dofile(g_L, path.string().c_str()) != LUA_OK) {
            const char* err = lua_tostring(g_L, -1);
            printf("[Lua] Error: %s\n", err);
            lua_pop(g_L, 1);
        }
    }
}

typedef void* (__thiscall* static_func_t)(char* self, const char*);
static_func_t script_func_orig;

typedef void* (__thiscall* register_t)(char* self, script_library_class*, const char*);
register_t script_func_reg_orig;

typedef bool(__cdecl* exec_t)(bool a1);
exec_t exec_orig;

typedef void(__cdecl* script_manager_run_t)(float unusedTime, bool run_once);
script_manager_run_t script_manager_run_orig;


struct vector3d {
    float x, y, z;
};

char* script_gsoi = (char*)(0x0096BB50);

BOOL __cdecl script_push_entity(void* a)
{
    return CDECL_CALL(0x0064E5C0, a);
}
BOOL __cdecl script_push_str(const char* a)
{
    return CDECL_CALL(0x0064E610, a);
}
BOOL __cdecl script_push_num(int a)
{
    return CDECL_CALL(0x0064E6A0, a);
}
BOOL __cdecl script_push_vector3d(vector3d* a)
{
    return static_cast<BOOL>(CDECL_CALL(0x0064E6D0, a));
}


int __cdecl script_find_function(string_hash a1, const char* a2, bool a3)
{
    return CDECL_CALL(0x0064E4F0, a1, a2, a3);
}
char* __cdecl script_new_thread(int a1, char* si)
{
    return (char*)CDECL_CALL(0x0064E520, a1, si);
}

bool push_vector3(lua_State* L, int index)
{
    if (!lua_istable(L, index))
        return false;

    vector3d vec{};
    lua_getfield(L, index, "x"); vec.x = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, index, "y"); vec.y = lua_tonumber(L, -1); lua_pop(L, 1);
    lua_getfield(L, index, "z"); vec.z = lua_tonumber(L, -1); lua_pop(L, 1);

    return script_push_vector3d(&vec);
}
struct vm_thread
{
    char pad[0x20 + 0x184];
    char* _SP;
};

int lua_script_dispatcher(lua_State* L)
{
    if (!lua_isstring(L, lua_upvalueindex(1)))
        return luaL_error(L, "Missing function name");

    const char* fn = lua_tostring(L, lua_upvalueindex(1));
    string_hash hash = {to_hash(fn)};
    
    int nargs = lua_gettop(L);
    for (int i = 1; i <= nargs; ++i) {

        if (lua_isinteger(L, i) || lua_isnumber(L, i)) {
            if (lua_isinteger(L, i)) {
                int val = lua_tointeger(L, i);
                if (!script_push_num(val))
                    return luaL_error(L, "Failed to push int %d", i);
            }
            else if (lua_isnumber(L, i)) {
                float val = static_cast<float>(lua_tonumber(L, i));
                if (!script_push_num(*reinterpret_cast<int*>(&val)))
                    return luaL_error(L, "Failed to push float %f", val);
            }
        }
        else if (lua_isstring(L, i)) {
            const char* str = lua_tostring(L, i);
            if (!script_push_str(str))
                return luaL_error(L, "Failed to push string arg %d", i);
        }
        else if (lua_istable(L, i)) {
            if (!push_vector3(L, i))
                return luaL_error(L, "Failed to push vector3 arg %d", i);
        }
        else {
            return luaL_error(L, "Unsupported arg type at index %d", i);
        }
    }

    /*
    vm_thread* thr = (vm_thread*)(0x0096BB54);
    char* sp_before = thr->_SP;
    
    char* sp_after = thr->_SP;

    const int nrets = (int)((uintptr_t)(sp_after - sp_before)) / 4;
    for (int i = 0; i < nrets; ++i)
        lua_pushinteger(L, *reinterpret_cast<int32_t*>(sp_before + (i * 4)));

    printf("executed engine func\n");
    return nrets;
    */
    return 0;
}

void register_func(const char* fn) {
    if (!fn) return;

    std::string name(fn);
    auto p = name.find('(');
    if (p != std::string::npos)
        name.resize(p);

    script_static_funcs.emplace_back(fn);
    printf("registering static func %s\n", fn);

    lua_getglobal(g_L, "engine");
    lua_pushstring(g_L, fn);
    lua_pushcclosure(g_L, lua_script_dispatcher, 1);
    lua_setfield(g_L, -2, name.c_str());              // engine[fn] = closure

    lua_pop(g_L, 1);
}
void register_class_func(script_library_class* cl, const char* fn) {
    if (!cl || !fn) return;

    std::string name(fn);
    auto p = name.find('(');
    if (p != std::string::npos)
        name.resize(p);

    script_class_funcs[cl->name].emplace_back(fn);
    printf("registering class func %s:%s\n", cl->name, fn);

    lua_getglobal(g_L, "engine");
    lua_getfield(g_L, -1, cl->name);
    if (!lua_istable(g_L, -1)) {
        lua_pop(g_L, 1);
        lua_newtable(g_L);
        lua_setfield(g_L, -2, cl->name);
        lua_getfield(g_L, -1, cl->name);
    }

    lua_pushstring(g_L, fn); 
    lua_pushcclosure(g_L, lua_script_dispatcher, 1);
    lua_setfield(g_L, -2, name.c_str());              // engine[class][fn] = closure

    lua_pop(g_L, 2);
}

void* __fastcall hk_script_func(char* self, void* edx, const char* a3)
{
    if (a3)
        register_func(a3);

    return script_func_orig(self, a3);
}
void* __fastcall hk_script_func_reg(char* self, void* edx, script_library_class* a2, const char* a3)
{
    if (a3) 
        register_class_func(a2, a3);

    return script_func_reg_orig(self, a2, a3);
}

void trigger_callback(const char* callback) {
    lua_getglobal(g_L, "engine");
    lua_getfield(g_L, -1, callback);
    if (lua_isfunction(g_L, -1)) {
        if (lua_pcall(g_L, 0, 0, 0) != LUA_OK) {
            const char* err = lua_tostring(g_L, -1);
            printf("Lua error in %s: %s\n", callback, err);
            lua_pop(g_L, 1);
        }
    }
    lua_pop(g_L, 1);

}

void init_scripting()
{
    if (g_L)
        return;

    g_L = luaL_newstate();
    luaL_openlibs(g_L);

    lua_newtable(g_L);
    lua_setglobal(g_L, "engine");
}

void hk_script_ctor()
{
    CDECL_CALL(0x0058F9C0);

    lua_getglobal(g_L, "engine");
    lua_pushnil(g_L);
    while (lua_next(g_L, -2)) {
        printf("engine.%s (type: %s)\n",
            lua_tostring(g_L, -2),
            luaL_typename(g_L, -1));
        lua_pop(g_L, 1);
    }

    enumerate_scripts();
}

void destroy_scripting()
{
    trigger_callback("on_shutdown");
    lua_close(g_L);
}

bool __cdecl hk_exec(bool a1)
{
    bool ret = exec_orig(a1);
    trigger_callback("on_script_exec");
    return ret;
}

void __cdecl hk_script_manager_run(float unusedTime, bool run_once)
{
    trigger_callback("tick");
    script_manager_run_orig(unusedTime, run_once);
}