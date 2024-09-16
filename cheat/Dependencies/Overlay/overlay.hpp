#pragma once
#include <Windows.h>
#include <d3dx9.h>
#include <d3d9.h>
#include <d3dx9tex.h>
#include <dwmapi.h>
#include "../../moneyhack.hpp"

#include "../Imgui/imgui_internal.h"
#include "../Imgui/imgui.h"
#include "../Imgui/imgui_impl_win32.h"
#include "../Imgui/imgui_impl_dx9.h"
#include "structs.h"

using namespace structs;

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "Dwmapi.lib")

IDirect3D9Ex* p_Object = NULL;
IDirect3DDevice9Ex* p_Device = NULL;
D3DPRESENT_PARAMETERS p_Params = { NULL };

HWND MyWnd = NULL;
HWND GameWnd = NULL;
MSG Message = { NULL };

RECT GameRect = { NULL };
D3DPRESENT_PARAMETERS d3dpp;

DWORD ScreenCenterX;
DWORD ScreenCenterY;

static ULONG G_Width = GetSystemMetrics(SM_CXSCREEN);
static ULONG G_Height = GetSystemMetrics(SM_CYSCREEN);

WPARAM main_loop();
void render();


auto init_wndparams(HWND hWnd) -> HRESULT
{
    if (FAILED(Direct3DCreate9Ex(D3D_SDK_VERSION, &p_Object)))
        exit(3);

    ZeroMemory(&p_Params, sizeof(p_Params));
    p_Params.Windowed = TRUE;
    p_Params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    p_Params.hDeviceWindow = hWnd;
    p_Params.MultiSampleQuality = D3DMULTISAMPLE_NONE;
    p_Params.BackBufferFormat = D3DFMT_A8R8G8B8;
    p_Params.BackBufferWidth = G_Width;
    p_Params.BackBufferHeight = G_Height;
    p_Params.EnableAutoDepthStencil = TRUE;
    p_Params.AutoDepthStencilFormat = D3DFMT_D16;
    p_Params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

    if (FAILED(p_Object->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &p_Params, 0, &p_Device))) {
        p_Object->Release();
        exit(4);
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGuiStyle& s = ImGui::GetStyle();
    io.IniFilename = NULL;
    
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX9_Init(p_Device);
    return S_OK;
}

auto cleanup_d3d() -> void
{
    if (p_Device != NULL) {
        p_Device->EndScene();
        p_Device->Release();
    }
    if (p_Object != NULL) {
        p_Object->Release();
    }
}

auto get_process_wnd(uint32_t pid) -> HWND
{
    std::pair<HWND, uint32_t> params = { 0, pid };
    BOOL bResult = EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        auto pParams = (std::pair<HWND, uint32_t>*)(lParam);
        uint32_t processId = 0;

        if (GetWindowThreadProcessId(hwnd, reinterpret_cast<LPDWORD>(&processId)) && processId == pParams->second) {
            SetLastError((uint32_t)-1);
            pParams->first = hwnd;
            return FALSE;
        }

        return TRUE;

        }, (LPARAM)&params);

    if (!bResult && GetLastError() == -1 && params.first)
        return params.first;

    return NULL;
}

auto set_window_target() -> void
{
    while (true) {
        GameWnd = get_process_wnd(Driver->Id);
        if (GameWnd) {
            ZeroMemory(&GameRect, sizeof(GameRect));
            GetWindowRect(GameWnd, &GameRect);
            DWORD dwStyle = GetWindowLong(GameWnd, GWL_STYLE);
            if (dwStyle & WS_BORDER)
            {
                GameRect.top += 32;
                G_Height -= 39;
            }
            ScreenCenterX = G_Width / 2;
            ScreenCenterY = G_Height / 2;
            MoveWindow(MyWnd, GameRect.left, GameRect.top, G_Width, G_Height, true);
            //Sleep(1);
        }
    }
}

auto setup_window() -> void
{
    //HANDLE Windowthread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)set_window_target, 0, 0, 0);
    //CloseHandle(Windowthread);

    WNDCLASSEXA wcex = {
        sizeof(WNDCLASSEXA),
        0,
        DefWindowProcA,
        0,
        0,
        nullptr,
        LoadIcon(nullptr, IDI_APPLICATION),
        LoadCursor(nullptr, IDC_ARROW),
        nullptr,
        nullptr,
        xorstr_("moneyhack"), // Use XOR obfuscation
        LoadIcon(nullptr, IDI_APPLICATION)
    };
    RECT Rect;

    GetWindowRect(GetDesktopWindow(), &Rect);

    RegisterClassExA(&wcex);

    MyWnd = CreateWindowExA(NULL, xorstr_("moneyhack"), xorstr_("moneyhack"), WS_POPUP, Rect.left, Rect.top, Rect.right, Rect.bottom, NULL, NULL, wcex.hInstance, NULL);
    SetWindowLong(MyWnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);
    SetLayeredWindowAttributes(MyWnd, RGB(0, 0, 0), 255, NULL);

    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(MyWnd, &margin);

    ShowWindow(MyWnd, SW_SHOW);
    UpdateWindow(MyWnd);
}

void DrawRect(int x, int y, int w, int h, RGBA* color, int thickness)
{
    ImGui::GetForegroundDrawList()->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), 0, 0, thickness);
}
