#pragma once

#include "pfx_interface.h"
#include "string_hash.h"

struct web_sound_params;

struct shared_sound_interface_info {
    web_sound_params *get_web_sound_params(string_hash a2);
};

struct sound_and_pfx_interface : pfx_interface {
    sound_and_pfx_interface();

    web_sound_params *get_web_sound_params(string_hash a1);

    //virtual
    void release_ifc();
};
