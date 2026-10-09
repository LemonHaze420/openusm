#include "debug_menu.h"

#include "mstring.h"
// #include "script_instance.h"
#include "script_object.h"
#include "string_hash.h"
#include "vm_executable.h"

#if STANDALONE_SYSTEM
#include "debug_menu_extra.h"
#include "debug_render.h"
#include "devopt.h"
#include "game.h"
#include "ngl.h"
#include "ngl_font.h"
#include "ngl_scene.h"
#include "os_developer_options.h"
#include "variables.h"
#include <dinput.h>
#endif

#include <algorithm>
#include <cassert>
#include <string>

#include <windows.h>

const char *to_string(debug_menu_entry_type entry_type)
{
    const char *strings[] = {
        "UNDEFINED", "FLOAT_E", "POINTER_FLOAT", "INTEGER", "POINTER_INT", "BOOLEAN_E", "POINTER_BOOL", "POINTER_MENU"};

    return strings[entry_type];
}

debug_menu_entry *g_debug_camera_entry{nullptr};
#if STANDALONE_SYSTEM
debug_menu *script_menu = nullptr;
debug_menu *progression_menu = nullptr;
#endif

void entry_frame_advance_callback_default([[maybe_unused]] debug_menu_entry *a1) {}

struct debug_menu;


std::string entry_render_callback_default(debug_menu_entry *entry)
{
    switch (entry->entry_type) {
    case FLOAT_E:
    case POINTER_FLOAT: {
        auto val = entry->get_fval();

        char str[64]{};
        snprintf(str, 64, "%.2f", val);
        return {str};
    }
    case BOOLEAN_E:
    case POINTER_BOOL: {
        bool val = entry->get_bval();

        auto *str = (val ? "True" : "False");
        return {str};
    }
    case INTEGER:
    case POINTER_INT: {
        auto val = entry->get_ival();

        char str[100]{};
        sprintf(str, "%d", val);
        return {str};
    }
    default:
        break;
    }

    return std::string{""};
}

typedef void (*menu_handler_function)(debug_menu_entry *, custom_key_type key_type);


debug_menu *current_menu = nullptr;

#if STANDALONE_SYSTEM
namespace {
bool menu_owns_pause = false;
constexpr DWORD menu_page_size = 18;
}
void close_debug()
{
    current_menu = nullptr;
    if (menu_owns_pause && g_game_ptr != nullptr)
        g_game_ptr->unpause();
    menu_owns_pause = false;
}

namespace {
bool remove_entry_from_menu(debug_menu *menu, debug_menu_entry *entry)
{
    if (menu == nullptr || entry == nullptr)
        return false;

    for (DWORD index = 0; index < menu->used_slots; ++index) {
        if (&menu->entries[index] == entry || menu->entries[index].script_source == entry) {
            for (DWORD next = index + 1; next < menu->used_slots; ++next)
                menu->entries[next - 1] = menu->entries[next];
            --menu->used_slots;
            menu->entries[menu->used_slots] = debug_menu_entry{};
            return true;
        }

        if (menu->entries[index].entry_type == POINTER_MENU &&
            remove_entry_from_menu(menu->entries[index].m_value.p_menu, entry))
            return true;
    }

    return false;
}
}  // namespace

void remove_debug_menu_entry(debug_menu_entry *entry)
{
    if (remove_entry_from_menu(debug_menu::root_menu, entry))
        return;
    if (remove_entry_from_menu(script_menu, entry))
        return;
    remove_entry_from_menu(progression_menu, entry);
}
#endif

void script_handler_helper(debug_menu_entry *a2)
{
    if (a2->field_18 >= 0 && a2->field_14 != nullptr) {
        auto *parent = a2->field_14->get_parent();
        auto *exe = parent->get_func(a2->field_18);
        assert(exe != nullptr);

        if (exe->get_parms_stacksize() == 4) {
            a2->field_14->add_thread(exe, (const char *)&a2);
        } else {
            a2->field_14->add_thread(exe);
        }

        debug_menu::hide();
    }
}

bool debug_menu_entry::set_script_handler(script_instance *inst, const mString &a3)
{
    auto *v14 = this;

    assert(inst != nullptr);

    auto *v3 = a3.c_str();
    string_hash v8{v3};

    auto *v5 = inst->get_parent();
    auto v9 = v5->find_func(v8);
    auto v13 = v9;

    bool result;
    if (v9 >= 0) {
        v14->field_14 = inst;
        v14->field_18 = v13;
        v14->m_game_flags_handler = script_handler_helper;
        result = true;
    } else {
        auto *v6 = a3.c_str();
        printf("Could not find handler: %s\n", v6);
        result = false;
    }

    return result;
}

debug_menu *debug_menu_entry::remove_menu()
{
    if (this->m_game_flags_handler != nullptr) {
        if (this->m_value.p_menu != nullptr) {
            this->m_value.p_menu->~debug_menu();
        }

        this->m_value.p_menu = nullptr;
        this->m_game_flags_handler(this);
    }

    return this->m_value.p_menu;
}

void debug_menu_entry::on_change(float a3, bool a4)
{
    printf("debug_menu_entry::on_change: text = %s, entry_type = %s, a5 = %d\n",
           this->text,
           to_string(this->entry_type),
           a4);

    switch (this->entry_type) {
    case FLOAT_E:
    case POINTER_FLOAT: {
        float v6;
        if (a4) {
            v6 = this->field_20.m_step_size * this->field_20.m_step_scale;
        } else {
            v6 = this->field_20.m_step_size;
        }

        auto v5 = this->get_fval() + a3 * v6;
        this->set_fval(v5, true);
        break;
    }
    case BOOLEAN_E:
    case POINTER_BOOL: {
        auto v3 = this->get_bval();
        this->set_bval(!v3, true);
        break;
    }
    case INTEGER:
    case POINTER_INT: {
        float v7 = (a4 ? this->field_20.m_step_size * this->field_20.m_step_scale : this->field_20.m_step_size);

        printf("%f\n", v7);
        auto v8 = std::abs(v7);
        if (v8 < 1.0) {
            v8 = 1.0;
        }

        auto v4 = this->get_ival();
        if (a3 >= 0.0) {
            this->set_ival((int)(v4 + v8), true);
        } else {
            this->set_ival((int)(v4 - v8), true);
        }

        break;
    }
    default:
        return;
    }
}

void debug_menu_entry::on_select(float a2)
{
    printf("debug_menu_entry::on_select: text = %s, entry_type = %s\n", this->text, to_string(this->entry_type));

    switch (this->entry_type) {
    case dUNDEFINED:
        if (this->m_game_flags_handler != nullptr) {
            this->m_game_flags_handler(this);
        }

        break;
    case BOOLEAN_E:
    case POINTER_BOOL:
        this->on_change(a2, false);
        break;
    case POINTER_MENU:
        this->remove_menu();
        if (this->m_value.p_menu != nullptr) {
            current_menu = this->m_value.p_menu;
        }

        break;
    default:
        return;
    }
}

void debug_menu_entry::set_submenu(debug_menu *submenu)
{
    this->entry_type = POINTER_MENU;
    this->m_value.p_menu = submenu;

    if (submenu != nullptr) {
        submenu->m_parent = current_menu;
    }
}

debug_menu_entry::debug_menu_entry(debug_menu *submenu) : entry_type(POINTER_MENU)
{
    m_value.p_menu = submenu;
    strncpy(this->text, submenu->title, MAX_CHARS_SAFE);
}

void *add_debug_menu_entry(debug_menu *menu, debug_menu_entry *entry)
{
    if (entry->entry_type == POINTER_MENU) {
        auto *submenu = entry->m_value.p_menu;
        if (submenu != nullptr) {
            submenu->m_parent = menu;
        }
    }

    if (menu->used_slots < menu->capacity) {
        void *ret = &menu->entries[menu->used_slots];
        memcpy(ret, entry, sizeof(debug_menu_entry));
        ++menu->used_slots;

        if (entry->entry_type == POINTER_MENU && menu->used_slots > 1) {
            std::swap(menu->entries[0], menu->entries[menu->used_slots - 1]);
        }

        if (menu->m_sort_mode != debug_menu::sort_mode_t::undefined) {
            auto begin = menu->entries;
            auto end = begin + menu->used_slots;
            auto find_it =
                std::find_if(begin, end, [](debug_menu_entry &entry) { return entry.entry_type != POINTER_MENU; });

            if (find_it != end) {
                auto sort = [mode = menu->m_sort_mode](debug_menu_entry &e0, debug_menu_entry &e1) {
                    auto v7 = e0.get_script_handler();
                    auto v2 = e1.get_script_handler();
                    if (mode == debug_menu::sort_mode_t::ascending) {
                        return v7 < v2;
                    } else {  //descending
                        return v7 > v2;
                    }
                };

                std::sort(begin, find_it, sort);

                std::sort(find_it, end, sort);
            }
        }

        return ret;
    } else {
        DWORD current_entries_size = sizeof(debug_menu_entry) * menu->capacity;
        DWORD new_entries_size = sizeof(debug_menu_entry) * EXTEND_NEW_ENTRIES;

        void *new_ptr = realloc(menu->entries, current_entries_size + new_entries_size);

        if (new_ptr == nullptr) {
            printf("RIP\n");
            __debugbreak();
        } else {
            menu->capacity += EXTEND_NEW_ENTRIES;
            menu->entries = static_cast<decltype(menu->entries)>(new_ptr);
            memset(static_cast<void *>(&menu->entries[menu->used_slots]), 0, new_entries_size);

            return add_debug_menu_entry(menu, entry);
        }
    }

    return nullptr;
}

void debug_menu::add_entry(debug_menu_entry *entry)
{
    add_debug_menu_entry(this, entry);
}

debug_menu *create_menu(const char *title, menu_handler_function function, DWORD capacity)
{
    auto *mem = malloc(sizeof(debug_menu));
    debug_menu *menu = new (mem) debug_menu{};

    strncpy(menu->title, title, MAX_CHARS_SAFE);

    menu->capacity = capacity;
    menu->handler = function;
    DWORD total_entries_size = sizeof(debug_menu_entry) * capacity;
    menu->entries = static_cast<decltype(menu->entries)>(malloc(total_entries_size));
    memset(static_cast<void *>(menu->entries), 0, total_entries_size);

    return menu;
}

debug_menu *create_menu(const char *title, debug_menu::sort_mode_t mode)
{
    const auto capacity = 100u;
    auto *mem = malloc(sizeof(debug_menu));
    debug_menu *menu = new (mem) debug_menu{};

    strncpy(menu->title, title, MAX_CHARS_SAFE);

    menu->capacity = capacity;
    DWORD total_entries_size = sizeof(debug_menu_entry) * capacity;
    menu->entries = static_cast<decltype(menu->entries)>(malloc(total_entries_size));
    memset(static_cast<void *>(menu->entries), 0, total_entries_size);

    menu->m_sort_mode = mode;

    return menu;
}

debug_menu_entry *create_menu_entry(const mString &str)
{
    auto *entry = new debug_menu_entry{str};
    return entry;
}

debug_menu_entry *create_menu_entry(debug_menu *menu)
{
    auto *entry = new debug_menu_entry{menu};
    return entry;
}

const char *to_string(custom_key_type key_type)
{
    if (key_type == ENTER) {
        return "ENTER";
    } else if (key_type == LEFT) {
        return "LEFT";
    } else if (key_type == RIGHT) {
        return "RIGHT";
    }

    return "";
}

void handle_game_entry(debug_menu_entry *entry, custom_key_type key_type)
{
    printf("handle_game_entry = %s, %s, entry_type = %s\n",
           entry->text,
           to_string(key_type),
           to_string(entry->entry_type));

    if (key_type == ENTER) {
        switch (entry->entry_type) {
        case dUNDEFINED: {
            if (entry->m_game_flags_handler != nullptr) {
                entry->m_game_flags_handler(entry);
            }
            break;
        }
        case BOOLEAN_E:
        case POINTER_BOOL: {
            auto v3 = entry->get_bval();
            entry->set_bval(!v3, true);
            break;
        }
        case POINTER_MENU: {
            if (entry->m_value.p_menu != nullptr) {
                current_menu = entry->m_value.p_menu;
            }
            return;
        }
        default:
            break;
        }
    } else if (key_type == LEFT) {
        entry->on_change(-1.0, false);
    } else if (key_type == RIGHT) {
        entry->on_change(1.0, true);
    }
}

#if STANDALONE_SYSTEM
namespace {
void populate_saved_settings(debug_menu_entry *entry)
{
    auto *menu = create_menu(entry->text);
    entry->set_submenu(menu);
    create_gamefile_menu(menu);
}

void add_game_toggle(debug_menu *menu, const char *name, unsigned id, bool value)
{
    debug_menu_entry entry{name};
    entry.set_id(id);
    entry.set_bval(value);
    entry.set_game_flags_handler(game_flags_handler);
    menu->add_entry(&entry);
}

void populate_game_menu(debug_menu_entry *entry)
{
    auto *menu = create_menu(entry->text);
    entry->set_submenu(menu);
    add_game_toggle(menu, "Physics Enabled", 0, g_game_ptr->is_physics_enabled());
    add_game_toggle(
        menu, "Slow Motion Enabled", 2, os_developer_options::instance->get_int(mString{"FRAME_LOCK"}) == 120);
    add_game_toggle(menu, "Show Districts", 6, os_developer_options::instance->get_flag(mString{"SHOW_STREAMER_INFO"}));
    add_game_toggle(
        menu, "Show Hero Position", 7, os_developer_options::instance->get_flag(mString{"SHOW_DEBUG_INFO"}));
    add_game_toggle(menu, "Show FPS", 8, os_developer_options::instance->get_flag(mString{"SHOW_FPS"}));
    debug_menu_entry saved{"Saved Game Settings"};
    saved.set_submenu(nullptr);
    saved.set_game_flags_handler(populate_saved_settings);
    menu->add_entry(&saved);
}

void add_lazy_menu(debug_menu *parent, const char *name, void (*populate)(debug_menu_entry *))
{
    debug_menu_entry entry{name};
    entry.set_submenu(nullptr);
    entry.set_game_flags_handler(populate);
    parent->add_entry(&entry);
}

void destroy_menu(debug_menu *menu)
{
    if (menu == nullptr)
        return;
    for (DWORD i = 0; i < menu->used_slots; ++i) {
        if (menu->entries[i].entry_type == POINTER_MENU)
            destroy_menu(menu->entries[i].m_value.p_menu);
    }
    free(menu->entries);
    free(menu);
}

void move_selection(int direction)
{
    auto &menu = *current_menu;
    if (!menu.used_slots)
        return;
    const auto selected = menu.window_start + menu.cur_index;
    const DWORD next =
        direction > 0 ? (selected + 1) % menu.used_slots : (selected + menu.used_slots - 1) % menu.used_slots;
    if (next < menu.window_start)
        menu.window_start = next;
    else if (next >= menu.window_start + menu_page_size)
        menu.window_start = next - menu_page_size + 1;
    menu.cur_index = next - menu.window_start;
}

void normalize_selection()
{
    auto &menu = *current_menu;
    if (!menu.used_slots) {
        menu.window_start = menu.cur_index = 0;
        return;
    }
    const DWORD selected = std::min(menu.window_start + menu.cur_index, menu.used_slots - 1);
    menu.window_start = std::min(menu.window_start, selected);
    if (selected >= menu.window_start + menu_page_size)
        menu.window_start = selected - menu_page_size + 1;
    menu.cur_index = selected - menu.window_start;
}

void format_entry(debug_menu_entry &entry, char (&text)[256])
{
    auto &source = entry.script_source != nullptr ? *entry.script_source : entry;
    const auto value = source.render_callback(&source);
    if (value.empty())
        snprintf(text, sizeof(text), "%s", source.text);
    else
        snprintf(text, sizeof(text), "%s: %s", source.text, value.c_str());
}
}

void debug_menu::init()
{
    if (root_menu != nullptr)
        return;
    root_menu = create_menu("Debug Menu");
    add_lazy_menu(root_menu, "Game", populate_game_menu);
    create_camera_menu_items(root_menu);
    create_warp_menu(root_menu);
    create_debug_district_variants_menu(root_menu);
    script_menu = create_menu("Script");
    progression_menu = create_menu("Progression");
    root_menu->add_entry(script_menu);
    root_menu->add_entry(progression_menu);

    auto *options = create_menu("Devopts");
    for (int i = 0; i < 226; ++i) {
        const auto *option = get_option(i);
        debug_menu_entry entry{option->m_name};
        if (option->m_type == game_option_t::FLAG_OPTION)
            entry.set_pt_bval(reinterpret_cast<bool *>(option->m_value.p_bval));
        else if (option->m_type == game_option_t::INT_OPTION) {
            entry.set_p_ival(option->m_value.p_ival);
            entry.set_min_value(-1000);
            entry.set_max_value(1000);
        }
        options->add_entry(&entry);
    }
    root_menu->add_entry(options);
    constexpr const char *render_names[]{"CAPSULE_HISTORY",
                                         "LIGHTS",
                                         "BOX_TRIGGERS",
                                         "WATER_EXCLUSION_TRIGGERS",
                                         "POINT_TRIGGERS",
                                         "ENTITY_TRIGGERS",
                                         "INTERACTABLE_TRIGGERS",
                                         "OCCLUSION",
                                         "LEGOS",
                                         "REGION_MESHES",
                                         "ENTITIES",
                                         "LOW_LODS",
                                         "ACTIVITY_INFO",
                                         "RENDER_INFO",
                                         "COLLIDE_INFO",
                                         "MARKERS",
                                         "PARKING_MARKERS",
                                         "WATER_EXIT_MARKERS",
                                         "MISSION_MARKERS",
                                         "PATHS",
                                         "GLASS_HOUSE",
                                         "OBBS",
                                         "TRAFFIC_PATHS",
                                         "MINI_GAME",
                                         "BRAINS",
                                         "VOICE",
                                         "PATROLS",
                                         "PAUSE_TIMERS",
                                         "ANIM_INFO",
                                         "SCENE_ANIM_INFO",
                                         "TARGETING",
                                         "VIS_SPHERES",
                                         "LADDERS",
                                         "COLLISIONS",
                                         "BRAINS_ENABLED",
                                         "ANCHORS",
                                         "LINE_INFO",
                                         "SUBDIVISION",
                                         "SKELETONS",
                                         "SOUND_STREAM_USAGE",
                                         "SPHERES",
                                         "LINES",
                                         "CYLINDERS",
                                         "DGRAPH",
                                         "PEDS",
                                         "TRAFFIC",
                                         "ALS",
                                         "AI_COVER_MARKERS",
                                         "LIMBO_GLOW",
                                         "BIPED_COLL_VOLUMES",
                                         "DECALS"};
    static_assert(std::size(render_names) == DEBUG_RENDER_ITEMS_COUNT);
    for (unsigned i = 0; i < std::size(render_names); ++i)
        debug_render_items_names()[i] = mString{render_names[i]};
    create_debug_render_menu(root_menu);
}

bool debug_menu_input(const char *keys)
{
    constexpr unsigned codes[]{DIK_INSERT, DIK_RETURN, DIK_ESCAPE, DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT};
    static unsigned held[std::size(codes)]{};
    static bool drain_keys = false;
    for (unsigned i = 0; i < std::size(codes); ++i)
        held[i] = keys[codes[i]] ? held[i] + 1 : 0;
    const bool any_held = std::any_of(std::begin(held), std::end(held), [](unsigned count) { return count != 0; });
    bool consumed = current_menu != nullptr || drain_keys;
    if (drain_keys) {
        drain_keys = any_held;
        return true;
    }
    if (g_game_ptr == nullptr)
        return consumed;
    const auto state = g_game_ptr->get_cur_state();
    if (current_menu != nullptr && state != game_state::RUNNING && state != game_state::PAUSED) {
        close_debug();
        drain_keys = any_held;
        return consumed;
    }
    if (held[0] == 1) {
        if (current_menu != nullptr) {
            close_debug();
            drain_keys = true;
        } else if (state == game_state::RUNNING || state == game_state::PAUSED) {
            debug_menu::init();
            const bool was_paused = g_game_ptr->flag.game_paused;
            if (!was_paused)
                g_game_ptr->pause();
            if (g_game_ptr->flag.game_paused) {
                menu_owns_pause = !was_paused;
                current_menu = debug_menu::root_menu;
                consumed = true;
            }
        }
        return consumed;
    }
    if (current_menu == nullptr)
        return consumed;
    normalize_selection();
    const auto repeat = [](unsigned count) {
        return count == 1 || (count >= 5 && count % 5 == 0);
    };
    if (held[2] == 1) {
        current_menu->go_back();
        drain_keys = current_menu == nullptr;
    } else if (repeat(held[4]))
        move_selection(1);
    else if (repeat(held[3]))
        move_selection(-1);
    else if (current_menu->used_slots) {
        auto &stored = current_menu->entries[current_menu->window_start + current_menu->cur_index];
        auto &entry = stored.script_source != nullptr ? *stored.script_source : stored;
        if (held[1] == 1) {
            if (entry.entry_type == POINTER_MENU && entry.m_game_flags_handler != nullptr) {
                destroy_menu(entry.m_value.p_menu);
                entry.m_value.p_menu = nullptr;
            }
            entry.on_select(1);
        } else if (held[5] == 1)
            entry.on_change(-1, false);
        else if (held[6] == 1)
            entry.on_change(1, true);
    }
    if (current_menu != nullptr && current_menu->used_slots) {
        normalize_selection();
        auto &stored = current_menu->entries[current_menu->window_start + current_menu->cur_index];
        auto &entry = stored.script_source != nullptr ? *stored.script_source : stored;
        entry.frame_advance_callback(&entry);
    }
    return true;
}

bool debug_menu_blocks_window_input(UINT message, WPARAM key)
{
    static bool captured_keys[256]{};
    if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
        auto &captured = captured_keys[key & 0xFFu];
        captured = captured || current_menu != nullptr;
        return captured;
    }
    if (message == WM_KEYUP || message == WM_SYSKEYUP) {
        auto &captured = captured_keys[key & 0xFFu];
        const bool consume = captured || current_menu != nullptr;
        captured = false;
        return consume;
    }
    if (message == WM_KILLFOCUS)
        std::fill_n(captured_keys, std::size(captured_keys), false);
    return current_menu != nullptr &&
           ((message >= WM_KEYFIRST && message <= WM_KEYLAST) || (message >= WM_MOUSEFIRST && message <= WM_MOUSELAST));
}

void debug_menu_render()
{
    if (current_menu == nullptr || nglSysFont() == nullptr || nglRootScene == nullptr)
        return;
    normalize_selection();
    auto *previous_scene = nglCurScene;
    nglCurScene = nglRootScene;
    const DWORD count = std::min(menu_page_size, current_menu->used_slots - current_menu->window_start);
    uint32_t width = 0, line_height = 0;
    nglGetStringDimensions(nglSysFont(), &width, &line_height, "%s", current_menu->title);
    char labels[menu_page_size][256];
    for (DWORD i = 0; i < count; ++i) {
        format_entry(current_menu->entries[current_menu->window_start + i], labels[i]);
        uint32_t row_width, row_height;
        nglGetStringDimensions(nglSysFont(), &row_width, &row_height, "%s", labels[i]);
        width = std::max(width, row_width);
        line_height = std::max(line_height, row_height);
    }
    nglQuad background;
    nglInitQuad(&background);
    nglSetQuadRect(&background, 20, 40, 20 + width + 24, 40 + line_height * (count + 3) + 18);
    nglSetQuadColor(&background, 0xBE0A0A0A);
    nglSetQuadZ(&background, 0.5f);
    nglListAddQuad(&background);
    float y = 52;
    nglListAddString(nglSysFont(), 28, y, 0.2f, 0xFF00FF00, 1, 1, "%s", current_menu->title);
    y += line_height;
    if (current_menu->window_start)
        nglListAddString(nglSysFont(), 28, y, 0.2f, 0xFFFF00FF, 1, 1, " ^ ^ ^ ");
    y += line_height;
    for (DWORD i = 0; i < count; ++i) {
        nglListAddString(
            nglSysFont(), 28, y, 0.2f, i == current_menu->cur_index ? 0xFFFFFF00 : 0xFFFFFFFF, 1, 1, "%s", labels[i]);
        y += line_height;
    }
    if (current_menu->window_start + count < current_menu->used_slots)
        nglListAddString(nglSysFont(), 28, y, 0.2f, 0xFFFF00FF, 1, 1, " v v v ");
    nglCurScene = previous_scene;
}
#endif
