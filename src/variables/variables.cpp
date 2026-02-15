#include "variables.h"

#include "../input.h"
#include "../inputsettings.h"

#include <dsound.h>

bool &byte_965950 = var<bool>(0x00965950);

RTL_CRITICAL_SECTION &g_CriticalSection = var<RTL_CRITICAL_SECTION>(0x009618F4);

PolytubeCustomMaterial *& webline_texture = var<PolytubeCustomMaterial *>(0x00958058);

float & flt_937FA4 = var<float>(0x00937FA4);

float & flt_937FF0 = var<float>(0x00937FF0);

float & flt_937FA8 = var<float>(0x00937FA8);

float & g_strafe_mult = var<float>(0x00921B70);

float & g_move_mult = var<float>(0x00921B6C);

float & sluggish_mix = var<float>(0x00959E44);

float & med_mix = var<float>(0x00959E50);

float & fast_mix = var<float>(0x00959E38);

float & pronto_mix = var<float>(0x00959E40);

float & slow_mix = var<float>(0x00959E4C);

float & nglIFLSpeed = var<float>(0x0093BC78);

float & s_camera_target_radius_factor = var<float>(0x0087EBC4);

float & flt_87EBD4 = var<float>(0x0087EBD4);

float & flt_86F860 = var<float>(0x0086F860);

int & dword_975308 = var<int>(0x00975308);

bool & byte_959561 = var<bool>(0x00959561);

int & dword_975314 = var<int>(0x00975314);

int & dword_97530C = var<int>(0x0097530C);

bool & byte_971F9C = var<bool>(0x00971F9C);

bool & s_freeze_game_time = var<bool>(0x009680AC);

bool & g_generating_vtables = var<bool>(0x0095A6F1);

float & g_tan_half_fov_ratio = var<float>(0x00921D7C);

LPDIRECTSOUNDBUFFER & dword_982570 = var<LPDIRECTSOUNDBUFFER>(0x00982570);

void *& dword_982574 = var<void *>(0x00982574);

bool & byte_965C20 = var<bool>(0x00965C20);

bool & g_distance_clipping_enabled = var<bool>(0x0095C718);

int & g_distance_clipping = var<int>(0x0095C2F8);

int & g_disable_occlusion_culling = var<int>(0x0095C7AC);

int & globalTextLanguage = var<int>(0x0096B430);

HWND & g_appHwnd = var<HWND>(0x009874C4);

bool & g_is_the_packer = var<bool>(0x009682E4);

bool & g_master_clock_is_up = var<bool>(0x00965EB0);

int & g_TOD = var<int>(0x0091E000);

char * (&dword_965C24)[14] = var<char *[14]>(0x00965C24);

char (&g_scene_name)[1024] = var<char[1024]>(0x0095C308);

bool & bExit = var<bool>(0x0095C178);

bool & byte_965BF9 = var<bool>(0x00965BF9);

HCURSOR & hCursor = var<HCURSOR>(0x0096596C);

HANDLE & hEvent = var<HANDLE>(0x00965C5C);

HANDLE & hObject = var<HANDLE>(0x00965C60);

bool & byte_965BF5 = var<bool>(0x00965BF5);
bool & byte_965BF6 = var<bool>(0x00965BF6);

int & dword_922908 = var<int>(0x00922908);

bool & byte_922994 = var<bool>(0x00922994);

#if !STANDALONE_SYSTEM
IDirect3DDevice9 *& g_Direct3DDevice = var<IDirect3DDevice9 *>(0x00971F94);

int & nWidth = var<int>(0x0093AE84);

int & nHeight = var<int>(0x0093AE88);

int & g_Windowed = var<int>(0x00946530);

HWND & g_hWnd = var<HWND>(0x00971F98);

bool & EnableShader = var<bool>(0x00972AB0);

#else
IDirect3DDevice9 *& g_Direct3DDevice = []() -> auto & {
    static IDirect3DDevice9 * g_Direct3DDevice1 {};
    return g_Direct3DDevice1;
}();

int & nWidth = []() -> auto & {
    static int g_nWidth {640};
    return g_nWidth;
}();

int & nHeight = []() -> auto & {
    static int g_nHeight {480};
    return g_nHeight;
}();

int & g_Windowed = []() -> auto & {
    static int g_Windowed1 {1};
    return g_Windowed1;
}();

HWND & g_hWnd = []() -> auto & {
    static HWND g_hWnd1 {};
    return g_hWnd1;
}();

bool & EnableShader = []() -> auto & {
    static bool g_EnableShader {false};
    return g_EnableShader;
}();
#endif

char & byte_965C21 = var<char>(0x00965C21);

char & byte_965BF8 = var<char>(0x00965BF8);

int & g_cx = var<int>(0x00965978);
int & g_cy = var<int>(0x00965968);

char *& dword_95C730 = var<char *>(0x0095C730);
char *& dword_95C72C = var<char *>(0x0095C72C);

int & dword_91E1D8 = var<int>(0x0091E1D8);

bool & g_indoors = var<bool>(0x0095C87B);

bool & g_player_shadows_enabled = var<bool>(0x00922C5C);
bool & g_enable_stencil_shadows = var<bool>(0x00921C98);

int & g_cur_shadow_target = var<int>(0x00965F44);

bool & byte_975468 = var<bool>(0x00975468);

char (&byte_9659B8)[260] = var<char[260]>(0x009659B8);

float & flt_965BDC = var<float>(0x00965BDC);

bool & ChromeEffect = var<bool>(0x0091E1D4);

float & flt_88E518 = var<float>(0x0088E518);

float & flt_88E51C = var<float>(0x0088E51C);

IDirectSound8 *& g_directSound = var<IDirectSound8 *>(0x00987518);

bool & cam_target_locked = var<bool>(0x0095C754);
