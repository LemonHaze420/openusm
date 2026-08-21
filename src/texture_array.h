#pragma once

struct nglTexture;
struct texture_to_frame_map;

struct texture_array {
    void load_map_internal(texture_to_frame_map *a2, const char *a3);

    static void load_map(texture_to_frame_map *a2, nglTexture *a1, const char *a3);

    static void unload_map(texture_to_frame_map *a1, nglTexture *a2);
};
