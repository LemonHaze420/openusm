#include "texture_resource_handler.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "osassert.h"
#include "resource_directory.h"
#include "utility.h"
#include "trace.h"
#include "worldly_pack_slot.h"

#include <cassert>

VALIDATE_SIZE(texture_resource_handler, 0x18);

texture_resource_handler::texture_resource_handler(worldly_pack_slot *a2)
{
    this->m_vtbl = 0x00888A28;
    this->my_slot = a2;
    this->field_10 = TLRESOURCE_TYPE_TEXTURE;
}

bool texture_resource_handler::_handle(worldly_resource_handler::eBehavior a2, limited_timer *a3)
{
    TRACE("texture_resource_handler::handle");

    return base_tl_resource_handler::_handle(a2, a3);
}

void texture_resource_handler::handle_resource_internal(tlresource_location *loc,
                                                        nglTextureFileFormat a3)
{
    TRACE("texture_resource_handler::handle_resource_internal", loc->get_name().to_string());

    if constexpr (1)
    {
        assert(loc != nullptr);

        auto *v4 = loc->get_name().to_string();

        auto *Tex = nglConstructTexture(tlFixedString{v4}, a3, loc->get_data(), loc->get_size());

        if (Tex == nullptr) {
            Tex = nglDefaultTex;
        }

        loc->set_data(bit_cast<char *>(Tex));
        if (a3 == 1)
        {
            if (Tex == nglDefaultTex) {
                error("ERROR: multipalette texture not found: %s", loc->get_name().to_string());
            }

            for (auto i = 0u; i < Tex->m_num_palettes; ++i)
            {
                auto &v19 = Tex->Frames[i]->FileName;

                tlresource_location *found_tlres_loc = nullptr;

                auto &dir = this->my_slot->get_resource_directory();

                auto found = dir.find_tlresource(v19.m_hash,
                                                 TLRESOURCE_TYPE_TEXTURE,
                                                 nullptr,
                                                 &found_tlres_loc);

                if (!found || found_tlres_loc == nullptr) {
                    auto *v7 = v19.to_string();
                    error("ERROR: multipalette sub-texture not found: %s", v7);
                }

                found_tlres_loc->set_data(bit_cast<char *>(&Tex->Frames[i]));
            }
        }

    }
    else
    {
        THISCALL(0x0056BBC0, this, loc, a3);
    }
}

void texture_resource_handler::_pre_handle_resources(worldly_resource_handler::eBehavior a2)
{
    TRACE("texture_resource_handler::pre_handle_resources");

    if constexpr (1)
    {
        if (a2 == LOAD)
        {
            auto &res_dir = this->my_slot->get_resource_directory();

            const auto size = res_dir.get_tlresource_count(TLRESOURCE_TYPE_TEXTURE);
            //sp_log("pre_handle_resources: %u", size);
            for (int i = 0; i < size; ++i)
            {
                auto *loc = res_dir.get_tlresource_location(i, TLRESOURCE_TYPE_TEXTURE);
                assert(loc != nullptr
                       //&& loc->get_size() >= 4
                );

                char *v4 = loc->get_data();
                if (v4[0] == 'D' && v4[1] == 'D' && v4[2] == 'S' && v4[3] == 'M') {
                    loc->sub_672BDD(14);
                } else if (v4[0] != 'D' || v4[1] != 'D' || v4[2] != 'S') {
                    if (v4[0] == 'D' && v4[1] == 'S' && v4[2] == 'M') {
                        loc->sub_672BDD(15);
                    } else {
                        loc->sub_672BDD(13);
                    }
                }

                //sp_log("%c %c %c", v4[0], v4[1], v4[2]);
            }

            this->field_14 = 0;
        }
    } else {
        THISCALL(0x00562FB0, this, a2);
    }
}

bool texture_resource_handler::_handle_resource(worldly_resource_handler::eBehavior behavior,
                                               tlresource_location *tlres_loc)
{
    TRACE("texture_resource_handler::handle_resource");

    if constexpr (1)
    {
        if (behavior == UNLOAD)
        {
            if (tlres_loc->get_type() != 15)
            {
                auto *tex = bit_cast<nglTexture *>(tlres_loc->get_data());
                if (tex != nullptr)
                {
                    if ((tex->field_34 & 2) == 0)
                    {
                        if (!nglCanReleaseTexture(tex)) {
                            return true;
                        }

                        nglDestroyTexture(tex);
                    }
                }
            }

            ++this->field_C;

        }
        else
        {
            auto v4 = this->field_14;
            if (v4 || tlres_loc->get_type() != 14)
            {
                if (v4 == 1 && tlres_loc->get_type() == TLRESOURCE_TYPE_TEXTURE) {
                    this->handle_resource_internal(tlres_loc, static_cast<nglTextureFileFormat>(0));
                } else if (v4 == 2 && tlres_loc->get_type() == 13) {
                    this->handle_resource_internal(tlres_loc, static_cast<nglTextureFileFormat>(3));
                }
            }
            else
            {
                this->handle_resource_internal(tlres_loc, static_cast<nglTextureFileFormat>(1));
            }

            ++this->field_C;
            auto *dir = &this->my_slot->get_resource_directory();
            if (this->field_C >= dir->get_tlresource_count(static_cast<tlresource_type>(1)))
            {
                if (auto v7 = this->field_14; v7 < 2) {
                    this->field_C = 0;
                    ++this->field_14;
                }
            }
        }
    }
    else
    {
        THISCALL(0x0056BB00, this, behavior, tlres_loc);
    }

    return false;
}

void texture_resource_handler_patch() {

    FUNC_ADDRESS(address, &texture_resource_handler::pre_handle_resources);
    //set_vfunc(0x00888A30, address);

    {
        FUNC_ADDRESS(address, &texture_resource_handler::_handle);
        set_vfunc(0x00888A2C, address);
    }

    {
        FUNC_ADDRESS(address, &texture_resource_handler::_handle_resource);
        set_vfunc(0x00888A34, address);
    }
}
