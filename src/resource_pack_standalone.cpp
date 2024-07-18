#include "resource_pack_standalone.h"

#include "common.h"
#include "func_wrapper.h"
#include "log.h"
#include "memory.h"
#include "mstring.h"
#include "cut_scene.h"
#include "nfl_system.h"
#include "os_file.h"
#include "parse_generic_mash.h"
#include "resource_directory.h"
#include "resource_manager.h"
#include "resource_key.h"
#include "trace.h"
#include "utility.h"
#include "variables.h"

#include <cassert>

VALIDATE_OFFSET(resource_pack_standalone, res_dir_mash, 0x38);
VALIDATE_OFFSET(resource_pack_standalone, base_offset, 0x3C);

resource_pack_standalone::resource_pack_standalone() : m_header()
{
    this->m_vtbl = 0x00889928;

    auto *header = &this->m_header;
    header->field_0 = {};

    header->field_14 = 0;
    header->directory_offset = 0;
    header->res_dir_mash_size = 0;
    header->field_20 = 0;
    header->field_24 = 0;
    header->field_28 = 0;

    this->res_dir = nullptr;
    this->res_dir_mash = nullptr;
    this->base_offset = 0;
    this->m_filedID = NFL_FILE_ID_INVALID;
    this->name.m_hash.source_hash_code = 0;
    this->name.m_type = RESOURCE_KEY_TYPE_NONE;
}

nflFileID resource_pack_standalone::get_nfl_file_handle() {
    return this->m_filedID;
}

bool resource_pack_standalone::get_unloaded_resource_location(const resource_key &a1,
                                                              resource_location *location_struct)
{
    assert(res_dir != nullptr);
    assert(location_struct != nullptr);

    resource_directory *find_dir = nullptr;
    resource_location *find_loc = nullptr;
    if (!this->res_dir->find_resource(a1, &find_dir, &find_loc))
    {
        return false;
    }

    *location_struct = *find_loc;
    location_struct->m_offset += this->base_offset;

    return true;
}

bool resource_pack_standalone::load(const mString &str)
{
    TRACE("resource_pack_standalone::load", str.c_str());

    bool result = false;

    if (this->m_filedID != NFL_FILE_ID_INVALID) {
        sp_log("Can't open standalone pack %s when we already have one loaded.", str.c_str());
        assert(0);
    }

    assert(this->res_dir == nullptr && this->res_dir_mash == nullptr);
    assert(this->base_offset == 0);
    assert(!name.is_set());

    if constexpr (1) {
        os_file file {mString{str.c_str()}, 1u};

        if (file.opened) {
            file.read(&this->m_header, sizeof(this->m_header));

            resource_key v12 = create_resource_key_from_path(str.c_str(), RESOURCE_KEY_TYPE_NONE);

            this->m_header.verify(v12);
            int res_dir_size = this->m_header.res_dir_mash_size;
            this->base_offset = res_dir_size;
            this->res_dir_mash = arch_memalign(16u, res_dir_size);

            assert(this->res_dir_mash != nullptr);
            assert(this->m_header.directory_offset != 0);

            file.set_fp(this->m_header.directory_offset, os_file::FP_BEGIN);
            {
                auto how_many_did_we_get = file.read(this->res_dir_mash, res_dir_size);
                assert(how_many_did_we_get == res_dir_size);
            }

            auto alloced_mem = parse_generic_object_mash(this->res_dir,
                                      this->res_dir_mash,
                                      nullptr,
                                      nullptr,
                                      nullptr,
                                      0,
                                      0,
                                      nullptr);
            assert(!alloced_mem && "This should NOT allocate anything!");

            this->res_dir->constructor_common(nullptr, nullptr, nullptr, 0u, 0u);
            file.close();

            mString v15 = mString {"data\\"} + str;

            this->m_filedID = nflOpenFile(1, v15.c_str());

            result = (this->m_filedID != NFL_FILE_ID_INVALID);
        }
    } else {
        result = THISCALL(0x0053E380, this, &str);
    }

    return result;
}

void resource_pack_standalone::unload() {
    if constexpr (0)
    {
        if (this->m_filedID.field_0 != -1) {
            nflCloseFile(this->m_filedID);
        }

        if (this->res_dir_mash != nullptr) {
            mem_freealign(this->res_dir_mash);
            this->res_dir_mash = nullptr;
            this->res_dir = nullptr;
        }

        this->m_header.clear();

        this->res_dir = nullptr;
        this->res_dir_mash = nullptr;
        this->base_offset = 0;
        this->m_filedID.field_0 = -1;
        this->name.m_hash.source_hash_code = 0;
        this->name.m_type = RESOURCE_KEY_TYPE_NONE;
    }
    else
    {
        THISCALL(0x0052ABB0, this);
    }
}

void sub_732D60(bool a1)
{
    if constexpr (1)
    {
        if (!resource_manager::using_amalga || a1) {
            cut_scene::stream_anim_pack.unload();
        }
    }
    else
    {
        CDECL_CALL(0x00732D60, a1);
    }
}


void resource_pack_standalone_patch() {

    SET_JUMP(0x00732D60, sub_732D60);

    {
        FUNC_ADDRESS(address, &resource_pack_standalone::load);
        SET_JUMP(0x0053E380, address);
    }
    return;

    {

        {
            FUNC_ADDRESS(address, &os_file::read);
            REDIRECT(0x0053E420, address);
            REDIRECT(0x0053E48D, address);
        }

        {
            FUNC_ADDRESS(address, &os_file::set_fp);
            REDIRECT(0x0053E47F, address);
        }

        REDIRECT(0x0053E437, create_resource_key_from_path);

        {
            FUNC_ADDRESS(address, &resource_pack_header::verify);
            REDIRECT(0x0053E441, address);
        }

        {
            FUNC_ADDRESS(address, &os_file::close);
            REDIRECT(0x0053E4C6, address);
        }

        {
            auto *address = &parse_generic_object_mash<resource_directory>;
            REDIRECT(0x0053E4A0, address);
        }

        REDIRECT(0x0053E46B, arch_memalign_internal);

        REDIRECT(0x0053E505, nflOpenFile);
    }
}
