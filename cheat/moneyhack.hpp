#pragma once
#include "xor.hpp"
#define IMGUI_DEFINE_MATH_OPERATORS
#define WIN32_MEAN_AND_LEAN
#include "Dependencies/Dependencies.h"
#include "Dependencies/Overlay/overlay.hpp"
#include "config.h"
#include "menu.hpp"
#include "cheat.hpp"


#pragma comment(lib, "winmm.lib")

namespace moneyhack
{
    inline void render();
    inline WPARAM main_loop();
    inline INT entry();

    inline bool T_STATUS;

    inline WPARAM main_loop()
    {
        menu::before();
        static RECT old_rc = { 0 };
        MSG Message = { 0 };
        SetWindowPos(MyWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;

        while (Message.message != WM_QUIT)
        {
            while (PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }

            if (storage::activewindow == GameWnd) {
                HWND hwndtest = GetWindow(storage::activewindow, GW_HWNDPREV);
                SetWindowPos(MyWnd, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            }

            POINT xy = { 0 };
            POINT p;
            GetCursorPos(&p);
            io.MousePos.x = p.x - xy.x;
            io.MousePos.y = p.y - xy.y;

            if (GetAsyncKeyState(0x1)) {
                io.MouseDown[0] = true;
                io.MouseClicked[0] = true;
                io.MouseClickedPos[0].x = io.MousePos.x;
                io.MouseClickedPos[0].y = io.MousePos.y;
            }
            else io.MouseDown[0] = false;

            RECT rc = { 0 };

            GetClientRect(GameWnd, &rc);
            ClientToScreen(GameWnd, &xy);
            rc.left = xy.x;
            rc.top = xy.y;

            if (memcmp(&rc, &old_rc, sizeof(RECT)) != 0)
            {
                old_rc = rc;
                storage::DISPLAYWIDTH = rc.right;
                storage::DISPLAYHEIGHT = rc.bottom;
                p_Params.BackBufferWidth = storage::DISPLAYWIDTH;
                p_Params.BackBufferHeight = storage::DISPLAYHEIGHT;
                SetWindowPos(MyWnd, nullptr, xy.x, xy.y, storage::DISPLAYWIDTH, storage::DISPLAYHEIGHT, SWP_NOREDRAW);
                p_Device->Reset(&p_Params);
            }
            render();
        }

        // Cleanup
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        cleanup_d3d();
        DestroyWindow(MyWnd);

        return Message.wParam;
    }
    inline void render()
    {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

         cheat::ESPLOOP();

        menu::rendermenu();

        ImGui::EndFrame();
        p_Device->SetRenderState(D3DRS_ZENABLE, FALSE);
        p_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        p_Device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        p_Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

        if (p_Device->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            p_Device->EndScene();
        }

        HRESULT result = p_Device->Present(nullptr, nullptr, nullptr, nullptr);

        if (result == D3DERR_DEVICELOST && p_Device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
        {
            ImGui_ImplDX9_InvalidateDeviceObjects();
            p_Device->Reset(&d3dpp);
            ImGui_ImplDX9_CreateDeviceObjects();
        }
    }

    inline INT entry()
    {
       T_STATUS = Driver->Attach(xorstr_(L"cs2.exe"));
       if (Driver->Id == NULL)
           T_STATUS = false;
     
       if (Driver->Driver == NULL)
           T_STATUS = false;
     
       storage::B_CLIENTDLL = Driver->GetModuleBase(xorstr_(L"client.dll"));
       storage::B_ENGINE2DLL = Driver->GetModuleBase(xorstr_(L"engine2.dll"));
       storage::B_SCENESYSTEMDLL = Driver->GetModuleBase(xorstr_(L"scenesystem.dll"));
       storage::cs2window = FindWindowW(xorstr_(L"SDL_app"), xorstr_(L"Counter-Strike 2"));
       GameWnd = storage::cs2window;
       if (storage::B_CLIENTDLL == NULL || storage::B_ENGINE2DLL == NULL || storage::B_SCENESYSTEMDLL == NULL || storage::cs2window == NULL)
           T_STATUS = false;
     
      if (T_STATUS == false)
      {
          AllocConsole();
          std::cout << "fail setup\n";
          return -1;
      }
      
        setup_window();
        init_wndparams(MyWnd);
        ShowWindow(GetConsoleWindow(), SW_HIDE);

        std::thread([&]() { cheat::CHEATMAIN(); }).detach();
        return main_loop();
    }
}
