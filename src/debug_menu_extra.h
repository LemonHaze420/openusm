#pragma once

struct debug_menu;
struct debug_menu_entry;

extern void create_camera_menu_items(debug_menu *parent);

extern void create_warp_menu(debug_menu *parent);

extern void create_debug_render_menu(debug_menu *parent);

extern void create_entity_animation_menu(debug_menu *parent);

extern void create_game_flags_menu(debug_menu *parent);

extern void game_flags_handler(debug_menu_entry *a1);
