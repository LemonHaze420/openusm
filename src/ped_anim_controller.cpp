#include "ped_anim_controller.h"

#include "common.h"
#include "nal_skeleton.h"

VALIDATE_SIZE(ped_anim_controller, 0x64u);

static const char *PED_ANIMTYPE_NAME = "Ped";

ped_anim_controller::ped_anim_controller(actor *a2, nalBaseSkeleton *new_skel, uint32_t a4,
                                         als::als_meta_anim_table_shared *a5)
    : nal_anim_controller(a2, new_skel, a4, a5)
{
    this->m_vtbl = 0x00881038;
    this->field_58 = this;
    this->field_54 = 0x00880BD0;
    this->field_60 = this;
    this->field_5C.m_vtbl = 0x00880BE8;

    assert(new_skel->GetAnimTypeName() == tlFixedString(PED_ANIMTYPE_NAME));
}
