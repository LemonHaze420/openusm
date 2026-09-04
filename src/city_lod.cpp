#include "city_lod.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "ngl_mesh.h"
#include "os_developer_options.h"
#include "parse_generic_mash.h"
#include "resource_key.h"
#include "resource_manager.h"
#include "trace.h"
#include "utility.h"

VALIDATE_SIZE(city_lod, 8u);
VALIDATE_SIZE(strip_lod, 0x18);

city_lod::city_lod(const char *a1)
{
    if constexpr (1) {
        this->field_4 = false;
        this->field_0 = nullptr;
        this->field_4 = os_developer_options::instance->get_flag(static_cast<os_developer_options::flags_t>(0xE));

        if (this->field_4) {
            resource_key resource_id{string_hash{a1}, RESOURCE_KEY_TYPE_LOD};

            int v1;
            auto *resource = resource_manager::get_resource(resource_id, &v1, nullptr);
            if (resource != nullptr) {
                parse_generic_object_mash(this->field_0, resource, nullptr, nullptr, nullptr, 0, 0, nullptr);
            }

            map_mesh() = nglGetMesh(tlFixedString{"lod_map000"}, true);

            sides_mesh() = nglGetMesh(tlFixedString{"lod_sides000"}, true);

            top_mesh() = nglGetMesh(tlFixedString{"lod_top000"}, true);

            top_texture() = nglGetTexture(tlFixedString{"SM2_CityLODroofs"});

            mString local_5c{a1};
            local_5c.append("_ground000");

            ground_mesh() = nglGetMesh(tlFixedString{local_5c.c_str()}, true);

            box_mesh() = nglGetFirstMeshInFile(tlFixedString{"bldg_box"});
            assert(box_mesh() != nullptr);

            for (auto uVar7 = 0u; uVar7 < box_mesh()->NSections; ++uVar7) {
                auto *Mat = box_mesh()->Sections[uVar7].Section->Material;

                if ((*Mat->Name) == tlFixedString{"BoxMatTop"}) {
                    static Var<nglMaterialBase *> dword_00956358{0x00956358};
                    dword_00956358() = Mat;
                }
            }
        }
    } else {
        THISCALL(0x00550420, this, a1);
    }
}

void city_lod::render()
{
    TRACE("city_lod::render");

    THISCALL(0x00540380, this);
}

void strip_lod::un_mash_start(generic_mash_header *a1, void *a2, generic_mash_data_ptrs *a3, void *a4)
{
    THISCALL(0x0052F380, this, a1, a2, a3, a4);
}

void city_lod_patch()
{
    {
        FUNC_ADDRESS(address, &city_lod::render);
        REDIRECT(0x0054B2FB, address);
    }
}
