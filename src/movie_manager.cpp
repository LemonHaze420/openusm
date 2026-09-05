#include "movie_manager.h"

#include "common.h"
#include "func_wrapper.h"
#include "ngl.h"
#include "variables.h"

#include <d3d9.h>
#include <windows.h>

#include <cstdio>

movie_manager::movie_manager() {}

bool movie_manager::load_and_play_movie(const char *name, const char *sound_name, bool wait_for_sound)
{
    if constexpr (STANDALONE_SYSTEM) {
        (void)sound_name;
        (void)wait_for_sound;

        struct BinkHandle {
            uint32_t width;
            uint32_t height;
            uint32_t frames;
            uint32_t current_frame;
        };

        using BinkOpenFn = BinkHandle *(__stdcall *)(const char *, uint32_t);
        using BinkCloseFn = void (__stdcall *)(BinkHandle *);
        using BinkWaitFn = int (__stdcall *)(BinkHandle *);
        using BinkDoFrameFn = int (__stdcall *)(BinkHandle *);
        using BinkNextFrameFn = void (__stdcall *)(BinkHandle *);
        using BinkCopyToBufferFn = int (__stdcall *)(
            BinkHandle *, void *, int, uint32_t, uint32_t, uint32_t, uint32_t);

        static HMODULE bink = LoadLibraryA("binkw32_.dll");
        if (bink == nullptr) {
            sp_log("Unable to load binkw32_.dll for movie playback");
            return false;
        }

        const auto bink_open = bit_cast<BinkOpenFn>(
            GetProcAddress(bink, MAKEINTRESOURCEA(38)));
        const auto bink_close = bit_cast<BinkCloseFn>(
            GetProcAddress(bink, MAKEINTRESOURCEA(16)));
        const auto bink_wait = bit_cast<BinkWaitFn>(
            GetProcAddress(bink, MAKEINTRESOURCEA(61)));
        const auto bink_do_frame = bit_cast<BinkDoFrameFn>(
            GetProcAddress(bink, MAKEINTRESOURCEA(23)));
        const auto bink_next_frame = bit_cast<BinkNextFrameFn>(
            GetProcAddress(bink, MAKEINTRESOURCEA(37)));
        const auto bink_copy_to_buffer = bit_cast<BinkCopyToBufferFn>(
            GetProcAddress(bink, MAKEINTRESOURCEA(18)));
        if (bink_open == nullptr || bink_close == nullptr || bink_wait == nullptr ||
            bink_do_frame == nullptr || bink_next_frame == nullptr ||
            bink_copy_to_buffer == nullptr) {
            sp_log("binkw32_.dll is missing a required movie export");
            return false;
        }

        char path[MAX_PATH];
        std::snprintf(path, sizeof(path), "data\\movies\\pc\\%s.dat", name);
        auto *movie = bink_open(path, 0);
        if (movie == nullptr) {
            sp_log("Unable to open movie %s", path);
            return false;
        }

        IDirect3DTexture9 *dx_texture = nullptr;
        const auto create_result = IDirect3DDevice9_CreateTexture(
            g_Direct3DDevice,
            movie->width,
            movie->height,
            1,
            D3DUSAGE_DYNAMIC,
            D3DFMT_X8R8G8B8,
            D3DPOOL_DEFAULT,
            &dx_texture,
            nullptr);
        if (FAILED(create_result) || dx_texture == nullptr) {
            bink_close(movie);
            sp_log("Unable to create movie texture for %s", path);
            return false;
        }

        nglTexture texture{};
        texture.m_width = movie->width;
        texture.m_height = movie->height;
        texture.m_d3d_format = D3DFMT_X8R8G8B8;
        texture.DXTexture = dx_texture;

        nglQuad quad{};
        nglInitQuad(&quad);
        nglSetQuadRect(&quad, 0.0f, 0.0f, 640.0f, 480.0f);
        nglSetQuadZ(&quad, 10.0f);
        nglSetQuadTex(&quad, &texture);

        bool skipped = false;
        DWORD last_frame_tick = GetTickCount();
        uint32_t last_frame_number = movie->current_frame;
        while (movie->current_frame != movie->frames) {
            MSG message;
            while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
                if (message.message == WM_QUIT) {
                    skipped = true;
                    break;
                }
                if ((message.message == WM_KEYDOWN ||
                     message.message == WM_SYSKEYDOWN) &&
                    (message.wParam == VK_ESCAPE ||
                     message.wParam == VK_RETURN ||
                     message.wParam == VK_SPACE)) {
                    skipped = true;
                    break;
                }
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
            if (skipped || (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0 ||
                (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0 ||
                (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0) {
                skipped = true;
                break;
            }
            if (GetTickCount() - last_frame_tick > 2000) {
                sp_log("Movie %s stopped advancing; skipping it.", path);
                skipped = true;
                break;
            }

            if (bink_wait(movie)) {
                Sleep(1);
                continue;
            }

            bink_do_frame(movie);
            D3DLOCKED_RECT locked_rect;
            if (SUCCEEDED(IDirect3DTexture9_LockRect(
                    dx_texture, 0, &locked_rect, nullptr, D3DLOCK_DISCARD))) {
                bink_copy_to_buffer(movie,
                                    locked_rect.pBits,
                                    locked_rect.Pitch,
                                    movie->height,
                                    0,
                                    0,
                                    3);
                IDirect3DTexture9_UnlockRect(dx_texture, 0);
            }

            nglListInit();
            nglSetClearFlags(7);
            nglSetClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            nglSetAspectRatio(1.0f);
            nglSetOrthoMatrix(1000.0f, 10000.0f);
            nglListAddQuad(&quad);
            nglListSend(true);
            bink_next_frame(movie);
            if (movie->current_frame != last_frame_number) {
                last_frame_number = movie->current_frame;
                last_frame_tick = GetTickCount();
            }
        }

        IDirect3DTexture9_Release(dx_texture);
        bink_close(movie);
        return skipped;
    } else {
        return static_cast<bool>(
            CDECL_CALL(0x006299E0, name, sound_name, wait_for_sound));
    }
}
