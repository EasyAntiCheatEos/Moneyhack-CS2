#include "moneyhack.hpp"
#include "img.h"
#include <thread>
#include "xor.hpp"
#include <map>
#include <deque>
namespace menu
{
    void rendermenu();
    IDirect3DTexture9* gun = nullptr;
    IDirect3DTexture9* eye = nullptr;
    bool shownmenu = false;
    std::deque<ImVec2> mouseTrail;
    const int maxTrailSize = 50;
    ImVec4 border_color = { 0.1529, 0.1529, 0.1490, 1.0000 };
    void before();
}
namespace custom
{
    bool Checkbox(const char* label, bool* v);
    void child(const char* name, ImVec2 size);
}
bool custom::Checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const ImVec2 test = ImVec2(2.0f, 0.f);
    const float check_size_y = label_size.y + test.y * 0.9f;
    const float text_offset_y = test.y;
    const float text_offset_x = test.x + 2.5; 
    const float check_sz = ImMin(check_size_y, check_size_y);
    const float pad = ImMax(1.0f, (float)(int)(check_sz / 10.0f));

    ImColor filledcolor = ImColor(50, 255, 50, 155);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + test.y * 0.6f, label_size.y + test.y * 0.6f));
    ImGui::ItemSize(check_bb, test.y);

    ImRect total_bb = check_bb;
    if (label_size.x > 0)
        ImGui::SameLine(0, style.ItemInnerSpacing.x);

    const ImRect text_bb(window->DC.CursorPos + ImVec2(text_offset_x, text_offset_y), window->DC.CursorPos + ImVec2(text_offset_x, text_offset_y) + label_size);

    if (label_size.x > 0)
    {
        ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), test.y);
        total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
    }

    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
        *v = !(*v);
    ImGui::GetStyle().FrameRounding = 0.5f;
    ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);

    if (*v)
    {
        window->DrawList->AddRectFilled(check_bb.Min + ImVec2(pad, pad), check_bb.Max - ImVec2(pad, pad), filledcolor, style.FrameRounding);
    }

    if (label_size.x > 0.0f)
        ImGui::RenderText(text_bb.GetTL(), label);

    return pressed;
}
void custom::child(const char* name, ImVec2 size)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg,menu::border_color);

    ImGui::BeginChild(name, size);
}
void menu::before()
{
    D3DXCreateTextureFromFileInMemory(p_Device, gun_P, sizeof(gun_P), &gun);
    D3DXCreateTextureFromFileInMemory(p_Device, eye_P, sizeof(eye_P), &eye);
}

void col()
{
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.FrameBorderSize = 0.0f;
    style.WindowRounding = 5.0f;
    style.Colors[ImGuiCol_WindowBg] = ImColor(5, 5, 5, 255);
    style.Colors[ImGuiCol_Button] = ImColor(20, 20, 20, 155);
    style.Colors[ImGuiCol_ButtonHovered] = ImColor(25, 25, 25, 155);
    style.Colors[ImGuiCol_ButtonActive] = ImColor(25, 25, 25, 155);
    style.Colors[ImGuiCol_FrameBg] = ImColor(50, 50, 50, 255);
    style.Colors[ImGuiCol_FrameBgHovered] = ImColor(70, 70, 70, 255);
    style.Colors[ImGuiCol_FrameBgActive] = ImColor(90, 90, 90, 255);
    style.Colors[ImGuiCol_Text] = ImColor(255, 255, 255, 255);
}

class GradientLine {
public:

    static bool Render(ImVec2 size)
    {
        static ImColor gradient_colors[] =
        {
            //https://txwes.libguides.com/c.php?g=978475&p=7075536

            //Red
            ImColor(255, 0, 0),
            //Yellow
            ImColor(255,255,0),
            //Lime
            ImColor(0, 255, 0),
            //Cyan / Aqua
            ImColor(0, 255, 255),
            //Blue
            ImColor(0, 0, 255),
            //Magenta / Fuchsia
            ImColor(255, 0, 255),
            //Red
            ImColor(255, 0, 0)
        };

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2      screen_pos = ImGui::GetCursorScreenPos();

        static int pos = 0;

        if (size.x - pos == 0)
            pos = 0;
        else
            pos++;

        for (int i = 0; i < 6; ++i)
        {
            ImVec2 item_spacing = ImGui::GetStyle().ItemSpacing;

            auto render = [&](int displacement)
                {
                    draw_list->AddRectFilledMultiColor
                    (
                        ImVec2((screen_pos.x - item_spacing.x - displacement) + (i) * (size.x / 6), (screen_pos.y - item_spacing.y)),
                        ImVec2((screen_pos.x - item_spacing.x + (item_spacing.x * 2) - displacement) + (i + 1) * (size.x / 6), (screen_pos.y - item_spacing.y) + (size.y)),

                        //add one to color to create a skuffed blend
                        gradient_colors[i], gradient_colors[i + 1], gradient_colors[i + 1], gradient_colors[i]
                    );
                };

            render((pos)); render((pos - size.x));
        }
        return true;
    }
};

void menu::rendermenu()
{
    if (GetAsyncKeyState(VK_INSERT) & 1)
    {
        shownmenu = !shownmenu;
    }

    static int TAB = 0;

    if (shownmenu)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mousePos = io.MousePos;

        if (menu::mouseTrail.size() >= menu::maxTrailSize)
        {
            menu::mouseTrail.pop_front();
        }
        menu::mouseTrail.push_back(mousePos);

        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        ImColor trailColor(0, 0, 255, 100);
        for (size_t i = 1; i < menu::mouseTrail.size(); ++i)
        {
            drawList->AddLine(menu::mouseTrail[i - 1], menu::mouseTrail[i], trailColor, 2.0f);
        }

        drawList->AddRectFilled(mousePos, ImVec2(mousePos.x + 5.f, mousePos.y + 5.f), ImColor(255, 255, 255));

        col();
        ImGui::SetNextWindowSize(ImVec2(400.0f, 300.0f), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);
        ImGui::Begin(xorstr_("##title"), nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
        GradientLine::Render(ImVec2(ImGui::GetContentRegionAvail().x * 2, 2));
        ImGui::Spacing();
       // ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.f);
        ImGui::BeginChild(xorstr_("##tabs"), ImVec2(60, 0), true, ImGuiWindowFlags_NoScrollbar);
        ImGui::Spacing();

        if (ImGui::ImageButton((void*)menu::eye, ImVec2(35, 35)))
        {
            TAB = 2;
        }

        ImGui::Spacing();
        ImGui::EndChild();
       // ImGui::PopStyleVar();

        ImGui::SameLine();
        ImGui::BeginChild(xorstr_("##content"), ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar);

        if (TAB == 2)
        {

            custom::Checkbox(xorstr_("3D Box"), &cfg.box3d);
            custom::Checkbox(xorstr_("Bone"), &cfg.SkellyBones);
            custom::Checkbox(xorstr_("Health"), &cfg.healthesp);
            custom::Checkbox(xorstr_("Snap Lines"), &cfg.SnapLines);
            ImGui::SeparatorEx(NULL, 1.f);
            ImGui::Spacing();

            ImGui::Text(xorstr_("Colors"));
            ImGui::ColorEdit4(xorstr_("3D Esp Box"), cfg.box3d_C, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
            ImGui::Spacing();
            ImGui::ColorEdit4(xorstr_("Bones"), cfg.SkellyBones_C, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_PickerHueWheel);
        }

        ImGui::EndChild();

        ImGui::End();
    }
}