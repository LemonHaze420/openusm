#include "glam_camera.h"

#include "wds.h"

//VALIDATE_SIZE(glam_camera, 0x1BCu);

glam_camera::glam_camera(const char *a1) : game_camera(string_hash{a1}, g_world_ptr->get_hero_ptr(0)) {}
