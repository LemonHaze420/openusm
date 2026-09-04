#include "camera_anim_controller.h"

#include "common.h"

#include "nal_skeleton.h"

VALIDATE_SIZE(camera_anim_controller, 0x64u);

static const char *CAMERA_ANIMTYPE_NAME = "Camera";

camera_anim_controller::camera_anim_controller(actor *a2, nalBaseSkeleton *new_skel, unsigned int a4,
                                               als::als_meta_anim_table_shared *a5)
    : nal_anim_controller(a2, new_skel, a4, a5)
{
    this->m_vtbl = 0x008810E0;
    this->field_58 = this;
    this->field_54.m_vtbl = 0x00880C14;
    this->field_60 = this;
    this->field_5C.m_vtbl = 0x00880C28;

    assert(new_skel->GetAnimTypeName() == tlFixedString(CAMERA_ANIMTYPE_NAME));
}
