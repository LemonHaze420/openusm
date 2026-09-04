#include <gtest/gtest.h>

#include <nal_system.h>
#include <nfl_system.h>
#include <nfl_driver.h>
#include <ngl.h>
#include <ngl_mesh.h>
#include <nglshader.h>
#include <os_developer_options.h>
#include <resource_manager.h>
#include <string_hash_dictionary.h>
#include <tl_system.h>
#include <variables.h>

TEST(ResourceManager, CreateInstance)
{
    g_platform = NL_PLATFORM_PC;

    initializeFileHandlers(nullptr);

    os_developer_options::os_developer_init();
    os_developer_options::instance->set_flag(mString{"SHOW_RESOURCE_SPAM"}, true);

    nflInitParams a1;
    a1.field_0[0] = 32;
    a1.field_0[1] = 32;
    a1.field_0[2] = 32;
    a1.field_0[0] = 1024;
    a1.field_0[3] = 3;
    a1.field_0[4] = 0;

    const auto dword_965C04 = nflInit(&a1);
    auto *dword_965C00 = new char[dword_965C04];

    nflStart(dword_965C00);

    string_hash_dictionary::create_inst();

    D3DPRESENT_PARAMETERS d3dpresent_params;
    d3dpresent_params.BackBufferWidth = nWidth;
    d3dpresent_params.BackBufferHeight = nHeight;
    d3dpresent_params.Windowed = g_Windowed;
    d3dpresent_params.BackBufferCount = 1;
    d3dpresent_params.BackBufferFormat = D3DFMT_A8R8G8B8;
    d3dpresent_params.MultiSampleType = D3DMULTISAMPLE_NONE;
    d3dpresent_params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpresent_params.hDeviceWindow = g_hWnd;
    d3dpresent_params.EnableAutoDepthStencil = true;
    d3dpresent_params.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpresent_params.Flags = 0;
    d3dpresent_params.PresentationInterval = 1;
    d3dpresent_params.FullScreen_RefreshRateInHz = (g_Windowed ? 0 : 60);

    auto *pD3D = Direct3DCreate9(0x80000020);

    g_valid_texture_format =
        IDirect3D9_CheckDeviceFormat(pD3D, 0, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_TEXTURE, D3DFMT_P8) < 0;

    ASSERT_TRUE(g_valid_texture_format);

    IDirect3D9_CreateDevice(pD3D,
                            D3DADAPTER_DEFAULT,
                            D3DDEVTYPE_REF,
                            g_hWnd,
                            D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                            &d3dpresent_params,
                            &g_Direct3DDevice);

    ASSERT_TRUE(g_Direct3DDevice != nullptr);

    nglMeshInit();

    nglTextureInit();
    tlInitListInit();

    nalInit(nullptr);

    set_tl_system_directories();

    resource_manager::create_inst();

    static tlSystemCallbacks ngl_callbacks{
        ngl_readfile_callback, ngl_releasefile_callback, 0, 0, ngl_memalloc_callback, ngl_memfree_callback};

    tlSetSystemCallbacks(ngl_callbacks);

    auto *tex = nglLoadTexture(tlFixedString{"c_alpha"});
    ASSERT_TRUE(tex != nullptr);

    ASSERT_TRUE(resource_manager::partitions != nullptr);
    ASSERT_TRUE(resource_manager::memory_maps != nullptr);
    ASSERT_TRUE(resource_manager::amalgapak_pack_location_table != nullptr);
    ASSERT_TRUE(resource_manager::amalgapak_prerequisite_table != nullptr);
    ASSERT_TRUE(resource_manager::resource_buffer != nullptr);
    ASSERT_EQ(resource_manager::in_use_memory_map, 0);
    ASSERT_EQ(resource_manager::amalgapak_base_offset, 0);
    ASSERT_EQ(resource_manager::using_amalga, false);
    ASSERT_EQ(resource_manager::amalgapak_pack_location_count, 618);
    ASSERT_EQ(std::string(resource_manager::amalgapak_name.c_str()), "packs\\amalga_PC.PAK");
    ASSERT_EQ(resource_manager::amalgapak_prerequisite_count, 617);
    ASSERT_EQ(resource_manager::resource_buffer_size, 36519936);
    ASSERT_EQ(resource_manager::memory_maps_count, 5);

    auto *common_partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_COMMON);
    ASSERT_NE(common_partition, nullptr);
    ASSERT_EQ(common_partition->get_pack_slots().size(), 1);
    ASSERT_NE(common_partition->get_streamer(), nullptr);

    {
        auto *start_partition = resource_manager::get_partition_pointer(RESOURCE_PARTITION_START);
        ASSERT_TRUE(start_partition != nullptr);
        ASSERT_NE(start_partition->get_streamer(), nullptr);
        ASSERT_EQ(start_partition->get_pack_slots().size(), 1);

        start_partition->get_streamer()->load("game", 0, nullptr, nullptr);
        start_partition->get_streamer()->flush(nullptr);

        auto *slot = start_partition->get_streamer()->get_pack_slots()->at(0);

        ASSERT_EQ(slot->get_resource_directory().parents.size(), 0);
        ASSERT_EQ(slot->get_resource_directory().resource_locations.size(), 112);
        ASSERT_EQ(slot->get_resource_directory().texture_locations.size(), 56);
        ASSERT_EQ(slot->get_resource_directory().mesh_file_locations.size(), 10);
        ASSERT_EQ(slot->get_resource_directory().mesh_locations.size(), 27);
        ASSERT_EQ(slot->get_resource_directory().morph_file_locations.size(), 0);
        ASSERT_EQ(slot->get_resource_directory().morph_locations.size(), 0);
        ASSERT_EQ(slot->get_resource_directory().material_file_locations.size(), 0);
        ASSERT_EQ(slot->get_resource_directory().material_locations.size(), 22);
        ASSERT_EQ(slot->get_resource_directory().anim_file_locations.size(), 0);
        ASSERT_EQ(slot->get_resource_directory().anim_locations.size(), 0);
        ASSERT_EQ(slot->get_resource_directory().scene_anim_locations.size(), 0);
        ASSERT_EQ(slot->get_resource_directory().skeleton_locations.size(), 7);
    }
}
