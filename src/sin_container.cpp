#include "sin_container.h"

#include "common.h"
#include "func_wrapper.h"
#include "utility.h"

VALIDATE_SIZE(sin_container, 0x30);
VALIDATE_OFFSET(sin_container, field_24, 0x24);

void sin_container::setup_world() {
    TRACE("sin_container::setup_world");
    sp_log("%s", this->master_script_name);
    sp_log("%s", this->field_4);

    THISCALL(0x0055BB00, this);
}

void sin_container::un_mash_start(generic_mash_header *a2,
                                  void *a3,
                                  generic_mash_data_ptrs *a4,
                                  void *) {
    this->un_mash(a2, a3, a4);
}

void sin_container::un_mash(generic_mash_header *a2, void *a3, generic_mash_data_ptrs *a4) {
    THISCALL(0x00520B00, this, a2, a3, a4);
}

mString sin_container::sub_55F530() {
    mString a2{this->master_script_name};
    return a2;
}

mString *sin_container::get_master_script_name(mString *out) {
    *out = {this->master_script_name};
    return out;
}

void sin_container_patch() {
    {
        FUNC_ADDRESS(address, &sin_container::setup_world);
        REDIRECT(0x0055CA45, address);
    }
}
