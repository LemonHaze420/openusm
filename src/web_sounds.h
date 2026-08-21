#pragma once

#include "string_hash.h"
#include "variable.h"

#include <list.hpp>

struct actor;
struct fixed_pool;
struct vector3d;

namespace web_sounds_manager {

void add_web_sound(actor *a1, const vector3d &a2, string_hash a3);
}

struct web_sound_params {};

struct web_sound {
    web_sound(web_sound_params *a2, actor *act, const vector3d &a4);

    void *operator new(size_t);

    void operator delete(void *);

    static inline fixed_pool &pool = *bit_cast<fixed_pool *>(0x00922108);
};

inline _std::list<web_sound *> *&s_web_sounds = *bit_cast<_std::list<web_sound *> **>(0x0095C870);
