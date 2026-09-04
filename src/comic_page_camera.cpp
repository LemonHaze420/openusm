#include "comic_page_camera.h"

#include "common.h"
#include "trace.h"
#include "vtbl.h"

VALIDATE_SIZE(comic_panels::page_camera, 0xF0);
VALIDATE_OFFSET(comic_panels::page_camera, field_3C, 0x3C);

namespace comic_panels {

Var<page_camera *> cur_page_camera{0x0096F7D8};

void *page_camera::CreateInstance(nalAnimClass<nalAnyPose> *a2)
{
    if constexpr (0) {
        return a2->VirtualCreateInstance(nullptr);
    } else {
        return (void *)THISCALL(0x00736F50, this, a2);
    }
}

void page_camera::Render(nalAnimClass<nalAnyPose>::nalInstanceClass *a1, Float a2)
{
    TRACE("page_camera::Render");

    if constexpr (1) {
    } else {
        THISCALL(0x00731D50, this, a1, a2);
    }
}

}  // namespace comic_panels
