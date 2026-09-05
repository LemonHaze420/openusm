#pragma once

#include <dsound.h>
#include <windows.h>

#include <cstdint>

// @todo: global config
struct GameConfig {
	bool DebugMode = false;

	bool WindowedMode = false;

	bool NoLoadScreen = false;
};

extern GameConfig g_config;


struct string_hash;
struct Input;
struct InputSettings;
struct PolytubeCustomMaterial;

#ifndef STANDALONE_SYSTEM
#define STANDALONE_SYSTEM 0
#endif

extern bool &byte_965950;

extern RTL_CRITICAL_SECTION &g_CriticalSection;

extern PolytubeCustomMaterial *& webline_texture;

extern float &flt_937FA4;

extern float &flt_937FF0;

extern float &flt_937FA8;

extern float &g_strafe_mult;

extern float &g_move_mult;

extern float &sluggish_mix;

extern float &med_mix;

extern float &fast_mix;

extern float &pronto_mix;

extern float &slow_mix;

extern float &nglIFLSpeed;

extern float &s_camera_target_radius_factor;

extern float &flt_87EBD4;

extern float &flt_86F860;

extern bool &byte_959561;

extern int &dword_975308;

extern int &dword_975314;

extern int &dword_97530C;

extern bool &byte_971F9C;

extern bool &s_freeze_game_time;

extern bool &g_generating_vtables;

extern LPDIRECTSOUNDBUFFER &dword_982570;

extern void *&dword_982574;

extern bool &byte_965C20;

extern bool &g_distance_clipping_enabled;

extern int &g_distance_clipping;

extern int &g_disable_occlusion_culling;

extern int &globalTextLanguage;

extern HWND &g_appHwnd;

extern bool &g_is_the_packer;

extern int &g_TOD;

extern char *(&dword_965C24)[14];

extern char (&g_scene_name)[1024];

extern bool &bExit;

extern bool &byte_965BF9;

extern HCURSOR &hCursor;

extern HANDLE &hEvent;

extern HANDLE &hObject;

extern bool &byte_965BF5;
extern bool &byte_965BF6;

struct IDirect3DDevice9;
extern IDirect3DDevice9 *&g_Direct3DDevice;

extern float &g_tan_half_fov_ratio;

extern int &dword_922908;

extern bool &byte_922994;

extern char &byte_965C21;

extern char &byte_965BF8;

extern int &g_cx;
extern int &g_cy;

extern HWND &g_hWnd;

extern char *&dword_95C730;
extern char *&dword_95C72C;

extern int &dword_91E1D8;

extern bool &g_indoors;

extern bool &g_player_shadows_enabled;
extern bool &g_enable_stencil_shadows;

extern int &g_cur_shadow_target;

extern char (&byte_9659B8)[260];

extern int &nWidth;
extern int &nHeight;

extern float &flt_965BDC;

extern bool &ChromeEffect;

extern int &g_Windowed;

struct IDirectSound8;

extern float &flt_88E518;

extern float &flt_88E51C;

extern IDirectSound8 *&g_directSound;

extern bool &g_master_clock_is_up;

extern bool &byte_975468;

extern bool &cam_target_locked;

extern bool &EnableShader;

inline constexpr uint32_t RESOURCE_VERSION_INVALID = 0xFFFFFFFF;

inline constexpr uint32_t RESOURCE_PACK_VERSION = 0xE;
inline constexpr uint32_t RESOURCE_ENTITY_MASH_VERSION = 0x24D;
inline constexpr uint32_t RESOURCE_NONENTITY_MASH_VERSION = 0x12D;
inline constexpr uint32_t RESOURCE_AUTO_MASH_VERSION = 0x249;
inline constexpr uint32_t RESOURCE_RAW_MASH_VERSION = 0x115;

inline constexpr auto flt_96A698 = 1.0 / 1024.0f;
