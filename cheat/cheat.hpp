#include "moneyhack.hpp"
#include <vector>
#include <thread>
#include <chrono>
#include <string>
#include <cstring>  // For std::strncpy
#include <format>   // For std::format (C++20)
 
bool w2s(const Vector3& pos, Vector3& out, view_matrix_t matrix)
{
    out.x = matrix[0][0] * pos.x + matrix[0][1] * pos.y + matrix[0][2] * pos.z + matrix[0][3];
    out.y = matrix[1][0] * pos.x + matrix[1][1] * pos.y + matrix[1][2] * pos.z + matrix[1][3];

    float w = matrix[3][0] * pos.x + matrix[3][1] * pos.y + matrix[3][2] * pos.z + matrix[3][3];

    if (w < 0.01f)
        return false;

    float inv_w = 1.0f / w;
    out.x *= inv_w;
    out.y *= inv_w;

    float x = storage::DISPLAYWIDTH * 0.5f;
    float y = storage::DISPLAYHEIGHT * 0.5f;

    x += 0.5f * out.x * storage::DISPLAYWIDTH + 0.5f;
    y -= 0.5f * out.y * storage::DISPLAYHEIGHT + 0.5f;

    out.x = x;
    out.y = y;

    return true;
}

namespace cheat
{
    int CHEATMAIN();
    void CHEATLOOP();
    void ESPLOOP();
}

struct PLAYER
{
    Vector3 POS;
    int Health;
    int Teamid;
    UINT flags;
    bool croching;
    std::string username;
    uintptr_t pl;
    std::vector<Vector3> bones;
};

std::vector<PLAYER> G_ENTITYLIST;

class LocalPlayer
{
public:
    int health;
    int teamid;
    int steamid;
    uintptr_t LocalPlayerPawn;

    void localplayercache()
    {
        LocalPlayerPawn = Driver->read<uintptr_t>(storage::B_CLIENTDLL + offsets::dwLocalPlayerPawn);
        health = Driver->read<int>(LocalPlayerPawn + offsets::client_dll::m_iHealth);
        teamid = Driver->read<int>(LocalPlayerPawn + offsets::client_dll::m_iTeamNum);
        this->steamid = Driver->read<INT>(this->LocalPlayerPawn + offsets::client_dll::m_steamID);
    }

    bool isdead() const
    {
        return health < 0;
    }

    bool isplayer() const
    {
        return LocalPlayerPawn != NULL;
    }

    bool hasteam() const
    {
        return teamid == 2 || teamid == 3;
    }

    Vector3 GetLocalPos()
    {
        return Driver->read<Vector3>(LocalPlayerPawn + offsets::client_dll::m_vOldOrigin);
    }
};

LocalPlayer LocalPl;

float distance(Vector3 p1, Vector3 p2)
{
    return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}

void EntityLoop()
{
    while (true)
    {
        std::vector<PLAYER> T_ENTLIST;
        uintptr_t entityList = Driver->read<uintptr_t>(storage::B_CLIENTDLL + offsets::dwEntityList);
        uintptr_t listEntry = Driver->read<uintptr_t>(entityList + 0x10);

        for (int i = 0; i < 64; ++i)
        {
            uintptr_t currentController = Driver->read<uintptr_t>(listEntry + i * 0x78);
            if (!currentController) continue;

            int pawnHandle = Driver->read<int>(currentController + offsets::m_hPlayerPawn);
            if (pawnHandle == 0) continue;

            uintptr_t listEntry2 = Driver->read<uintptr_t>(entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 0x10);
            
            uintptr_t entityPawn = Driver->read<uintptr_t>(listEntry2 + 0x78 * (pawnHandle & 0x1FF));

            if (!entityPawn) continue;

            int health = Driver->read<int>(entityPawn + offsets::client_dll::m_iHealth);
            if (health <= 0) continue;

            int teamId = Driver->read<int>(entityPawn + offsets::client_dll::m_iTeamNum);
            if (teamId == LocalPl.teamid) continue;
            if (teamId == NULL)
                continue;

            UINT flags = Driver->read<UINT>(entityPawn + 0x3CC);

            Vector3 position = Driver->read<Vector3>(entityPawn + offsets::client_dll::m_vOldOrigin);
            uint64_t gamescene = Driver->read<uint64_t>(entityPawn + 0x308);
            uint64_t bonearray = Driver->read<uint64_t>(gamescene + 0x170 + 0x80);

            Vector3 bones_E[] = {
                Driver->read<Vector3>(bonearray + 6 * 32),
                Driver->read<Vector3>(bonearray + 5 * 32),
                Driver->read<Vector3>(bonearray + 8 * 32),
                Driver->read<Vector3>(bonearray + 13 * 32),
                Driver->read<Vector3>(bonearray + 9 * 32),
                Driver->read<Vector3>(bonearray + 14 * 32),
                Driver->read<Vector3>(bonearray + 11 * 32),
                Driver->read<Vector3>(bonearray + 16 * 32),
                Driver->read<Vector3>(bonearray + 0 * 32),
                Driver->read<Vector3>(bonearray + 23 * 32),
                Driver->read<Vector3>(bonearray + 26 * 32),
                Driver->read<Vector3>(bonearray + 24 * 32),
                Driver->read<Vector3>(bonearray + 27 * 32)
            };

            PLAYER ent;
            ent.POS = position;
            ent.Teamid = teamId;
            ent.Health = health;
            ent.flags = flags;
            if (flags == 65667)
            {
                ent.croching = true;
            }
            else
            {
                ent.croching = false;
            }
            ent.pl = entityPawn;
            ent.bones.assign(bones_E, bones_E + 13);

            T_ENTLIST.push_back(ent);
        }

        G_ENTITYLIST = T_ENTLIST;
    }
}

ImU32 GetColorHealth(int health)
{
    if (health > 75)
        return ImColor(
            static_cast<float>(0) / 255.0f,
            static_cast<float>(255) / 255.0f,
            static_cast<float>(0) / 255.0f,
            0.8f
        ); // Green
    if (health > 50)
        return ImColor(
            static_cast<float>(255) / 255.0f,
            static_cast<float>(255) / 255.0f,
            static_cast<float>(0) / 255.0f,
            0.8f
        ); // Yellow
    if (health > 25)
        return ImColor(
            static_cast<float>(255) / 255.0f,
            static_cast<float>(127) / 255.0f,
            static_cast<float>(0) / 255.0f,
            0.8f
        ); // Orange
    if (health > 0)
        return ImColor(
            static_cast<float>(255) / 255.0f,
            static_cast<float>(0) / 255.0f,
            static_cast<float>(0) / 255.0f,
            0.8f
        ); // Red
    return ImColor(
        static_cast<float>(127) / 255.0f,
        static_cast<float>(127) / 255.0f,
        static_cast<float>(127) / 255.0f,
        0.8f
    ); // Gray
}

template <typename T>
T custom_min(const T& a, const T& b) {
    return (a < b) ? a : b;
}

template <typename T>
T custom_max(const T& a, const T& b) {
    return (a > b) ? a : b;
}

void cheat::ESPLOOP() {
    view_matrix_t viewMatrix = Driver->read<view_matrix_t>(storage::B_CLIENTDLL + offsets::dwViewMatrix);
    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    if (cfg.fovcircle_B)
    {
        drawList->AddCircle(storage::center, cfg.fovcircle_S, IM_COL32(255, 255, 255, 255), 300, 1.0f);
    }


    for (const PLAYER& player : G_ENTITYLIST) {
        Vector3 head_t = { player.POS.x, player.POS.y, player.POS.z + 75.0f };
        Vector3 screenPos, screenHead;

        if (!w2s(player.POS, screenPos, viewMatrix) || !w2s(head_t, screenHead, viewMatrix))
            continue;

        Vector3 head_GH = player.bones[1];
        head_GH.z += 6.5;
        Vector3 projectHead = head_GH.world_to_screen(viewMatrix);

        float height = abs(screenPos.y - screenHead.y);
        float width = height / 2.0f;

        float boxHeight = player.croching ? 55.0f : 75.0f;
        Vector3 min = { player.POS.x - 15.0f, player.POS.y - 15.0f, player.POS.z };
        Vector3 max = { player.POS.x + 15.0f, player.POS.y + 15.0f, player.POS.z + boxHeight };

        Vector3 points[8] = {
            { min.x, min.y, min.z }, { max.x, min.y, min.z },
            { max.x, max.y, min.z }, { min.x, max.y, min.z },
            { min.x, min.y, max.z }, { max.x, min.y, max.z },
            { max.x, max.y, max.z }, { min.x, max.y, max.z }
        };

        Vector3 screenPoints[8];
        bool render = true;

        for (int i = 0; i < 8; ++i) {
            if (!w2s(points[i], screenPoints[i], viewMatrix)) {
                render = false;
                break;
            }
        }

        if (!render)
            continue;

        bool isOffScreen = true;
        for (int i = 0; i < 8; i++) {
            if (screenPoints[i].x >= 0.0f && screenPoints[i].x <= storage::DISPLAYWIDTH &&
                screenPoints[i].y >= 0.0f && screenPoints[i].y <= storage::DISPLAYHEIGHT) {
                isOffScreen = false;
                break;
            }
        }

        if (isOffScreen)
            continue;

        if (cfg.box3d) {
            for (int i = 0; i < 4; ++i) {
                drawList->AddLine(ImVec2(screenPoints[i].x, screenPoints[i].y), ImVec2(screenPoints[(i + 1) % 4].x, screenPoints[(i + 1) % 4].y), ImGui::GetColorU32(ImVec4(cfg.box3d_C[0], cfg.box3d_C[1], cfg.box3d_C[2], cfg.box3d_C[3])), cfg.box3dthickness);
                drawList->AddLine(ImVec2(screenPoints[i + 4].x, screenPoints[i + 4].y), ImVec2(screenPoints[((i + 1) % 4) + 4].x, screenPoints[((i + 1) % 4) + 4].y), ImGui::GetColorU32(ImVec4(cfg.box3d_C[0], cfg.box3d_C[1], cfg.box3d_C[2], cfg.box3d_C[3])), cfg.box3dthickness);
                drawList->AddLine(ImVec2(screenPoints[i].x, screenPoints[i].y), ImVec2(screenPoints[i + 4].x, screenPoints[i + 4].y), ImGui::GetColorU32(ImVec4(cfg.box3d_C[0], cfg.box3d_C[1], cfg.box3d_C[2], cfg.box3d_C[3])), cfg.box3dthickness);
            }
        }

        if (cfg.SkellyBones) {
            Vector3 screenBones[13];
            for (int i = 0; i < 13; ++i) {
                if (!w2s(player.bones[i], screenBones[i], viewMatrix))
                    continue;
            }

            ImVec2 boneScreen[13];
            for (int i = 0; i < 13; ++i) {
                boneScreen[i] = ImVec2(screenBones[i].x, screenBones[i].y);
            }

            auto drawBoneLine = [&](int a, int b) {
                if (a < 13 && b < 13)
                    drawList->AddLine(boneScreen[a], boneScreen[b], ImGui::GetColorU32(ImVec4(cfg.SkellyBones_C[0], cfg.SkellyBones_C[1], cfg.SkellyBones_C[2], cfg.SkellyBones_C[3])), cfg.skellbonesthickness);
                };

            drawBoneLine(0, 1);
            drawBoneLine(1, 2);
            drawBoneLine(1, 3);
            drawBoneLine(2, 4);
            drawBoneLine(3, 5);
            drawBoneLine(4, 6);
            drawBoneLine(5, 7);
            drawBoneLine(1, 8);
            drawBoneLine(8, 9);
            drawBoneLine(8, 10);
            drawBoneLine(9, 11);
            drawBoneLine(10, 12);
            ImGui::GetForegroundDrawList()->AddCircle(ImVec2(projectHead.x, projectHead.y), height / 10, ImColor(255,255,255,255), 100);
        }

        if (cfg.healthesp || cfg.nameesp || cfg.SnapLines) {
            ImVec2 least(screenPoints[0].x, screenPoints[0].y), furthest(screenPoints[0].x, screenPoints[0].y);

            for (int i = 1; i < 8; i++) {
                least.x = custom_min(least.x, screenPoints[i].x);
                least.y = custom_min(least.y, screenPoints[i].y);
                furthest.x = custom_max(furthest.x, screenPoints[i].x);
                furthest.y = custom_max(furthest.y, screenPoints[i].y);
            }

            if (cfg.healthesp) {
                float healthBarWidth = 5.0f;
                float healthBarHeight = (furthest.y - least.y) * (player.Health / 100.0f);


                drawList->AddRectFilled(
                    ImVec2(least.x - healthBarWidth - 2.0f, least.y),
                    ImVec2(least.x - 2.0f, least.y + healthBarHeight),
                    GetColorHealth(player.Health)
                );
            }

          // if (cfg.nameesp) {
          //     ImVec2 textSize = ImGui::CalcTextSize(player.username.c_str());
          //     ImVec2 textPos = ImVec2(least.x + (furthest.x - least.x) / 2.0f - textSize.x / 2.0f, least.y - textSize.y - 2.0f);
          //     drawList->AddText(textPos, ImColor(255, 255, 255, 255), player.username.c_str());
          // }


            if (cfg.SnapLines) {
                Vector3 neckBone = player.bones[1];
                Vector3 screenNeckBone;
                if (w2s(neckBone, screenNeckBone, viewMatrix)) {
                    ImVec2 centerOfScreen = ImVec2(storage::DISPLAYWIDTH / 2.0f, storage::DISPLAYHEIGHT / 2.0f);
                    drawList->AddLine(ImVec2(screenNeckBone.x, screenNeckBone.y), centerOfScreen, ImGui::GetColorU32(ImVec4(cfg.snapline_C[0], cfg.snapline_C[1], cfg.snapline_C[2], cfg.snapline_C[3])), cfg.snaplinethickness);
                }
            }
        }
    }
}



void fast()
{
    while (1)
    {
        int entindex = Driver->read<INT>(LocalPl.LocalPlayerPawn + 0x13A8);
        if (GetAsyncKeyState(VK_XBUTTON2))
        {
            if (entindex > 0)
            {
                INPUT input = { 0 };
                input.type = INPUT_MOUSE;
                input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
                SendInput(1, &input, sizeof(INPUT));

                input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
                SendInput(1, &input, sizeof(INPUT));
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }
    }
}

void cheat::CHEATLOOP()
{
    while (true)
    {
        LocalPl.localplayercache();
        storage::activewindow = GetForegroundWindow();
        storage::B_CLIENTDLL = Driver->GetModuleBase(L"client.dll");
        if (storage::B_CLIENTDLL == NULL)
        {
            exit(1);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}


int cheat::CHEATMAIN()
{
    std::thread(cheat::CHEATLOOP).detach();
    std::thread(EntityLoop).detach();
    std::thread(fast).detach();
    return 1;
}
