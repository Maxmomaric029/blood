#include "gui.h"
#include "globals.h"
#include "hacks.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <windows.h>
#include <cmath>
#include <cstdlib>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

// Particles
struct Particle {
    ImVec2 pos;
    ImVec2 velocity;
    float size;
    float alpha;
    float life;
};
std::vector<Particle> particles;

void UpdateParticles() {
    ImVec2 screenSize = ImGui::GetWindowSize();
    if (particles.size() < 50) {
        Particle p;
        p.pos = ImVec2((float)(rand() % (int)screenSize.x), (float)(rand() % (int)screenSize.y));
        p.velocity = ImVec2(((rand() % 100) - 50) * 0.05f, ((rand() % 100) - 50) * 0.05f);
        p.size = (rand() % 3) + 1.0f;
        p.alpha = 1.0f;
        p.life = 100.0f;
        particles.push_back(p);
    }

    for (auto it = particles.begin(); it != particles.end(); ) {
        it->pos.x += it->velocity.x;
        it->pos.y += it->velocity.y;
        it->alpha -= 0.005f;
        
        if (it->alpha <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

void DrawParticles() {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();
    for (auto& p : particles) {
        drawList->AddCircleFilled(ImVec2(winPos.x + p.pos.x, winPos.y + p.pos.y), p.size, ImColor(255, 0, 0, (int)(p.alpha * 255)));
    }
}

// Custom Toggle Button
bool ToggleButton(const char* label, bool* v) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    float height = ImGui::GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    float width = ImGui::GetContentRegionAvail().x;
    const ImRect bb(pos, ImVec2(pos.x + width, pos.y + height));
    
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);
    if (pressed) *v = !(*v);

    // Render
    float t = *v ? 1.0f : 0.0f;
    ImU32 col_bg = ImGui::GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    if (*v) col_bg = IM_COL32(180, 0, 0, 200); // Red Active

    window->DrawList->AddRectFilled(bb.Min, bb.Max, col_bg, 4.0f);
    
    // Text
    ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    
    // Status dot
    ImVec2 dotPos = ImVec2(bb.Max.x - 20, bb.Min.y + height/2);
    window->DrawList->AddCircleFilled(dotPos, 4.0f, *v ? IM_COL32(0, 255, 0, 255) : IM_COL32(100, 100, 100, 255));

    return pressed;
}

void InjectDLL(std::string url, std::string path) {
    HRESULT hr = URLDownloadToFileA(NULL, url.c_str(), path.c_str(), 0, NULL);
    if (SUCCEEDED(hr)) {
        // Simple mock injection call - in real scenario, use LoadLibrary injection code from DllInjector.h
        // For this optimized version, we assume user just wants the download + execution or manual inject.
        // We will just log it for now as the user asked for S/R mainly.
    }
}

void SetupStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.90f); // Dark Transparent
    style.Colors[ImGuiCol_Border] = ImVec4(0.8f, 0.0f, 0.0f, 0.5f); // Red Border
    style.WindowRounding = 8.0f;
    style.FrameRounding = 4.0f;
}

void RenderGUI() {
    UpdateParticles();
    
    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_Once);
    ImGui::Begin("BLOODIE HACK | OPTIMIZED", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
    
    DrawParticles();

    // -- Header --
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "BLOODIE HACK");
    ImGui::SameLine();
    ImGui::TextDisabled("v2.0 Optimized");
    ImGui::Separator();

    // -- Login --
    if (!Globals::isLoggedIn) {
        ImGui::SetCursorPos(ImVec2(150, 150));
        ImGui::BeginGroup();
        ImGui::Text("Enter Key:");
        ImGui::InputText("##key", Globals::licenseKey, 256);
        if (ImGui::Button("LOGIN", ImVec2(300, 40))) {
            if (std::string(Globals::licenseKey) == "bloodie") {
                Globals::isLoggedIn = true;
            }
        }
        ImGui::EndGroup();
        ImGui::End();
        return;
    }

    // -- Status --
    ImGui::Text("Emulator:"); ImGui::SameLine();
    if (Globals::isAttached) ImGui::TextColored(ImVec4(0,1,0,1), "%s", Globals::currentProcess.c_str());
    else ImGui::TextColored(ImVec4(1,0,0,1), "Not Found");

    // -- Tabs --
    ImGui::Spacing();
    if (ImGui::Button("AIMBOT", ImVec2(190, 30))) Globals::activeTab = 0; ImGui::SameLine();
    if (ImGui::Button("MISC", ImVec2(190, 30))) Globals::activeTab = 1; ImGui::SameLine();
    if (ImGui::Button("SETTINGS", ImVec2(190, 30))) Globals::activeTab = 2;
    ImGui::Separator();

    // -- Content --
    ImGui::BeginChild("Content", ImVec2(0, 0), true);
    
    if (Globals::activeTab == 0) { // AIMBOT
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "[RISK] USE AT OWN RISK");
        ToggleButton("Aim Lock (Head)", &Globals::bAimLock);
        ToggleButton("Aim Head (OB52)", &Globals::bAimbotHead);
        
        ImGui::Spacing();
        ImGui::Text("Settings");
        ImGui::SliderFloat("FOV", &Globals::fFov, 10.0f, 360.0f);
        ImGui::SliderFloat("Sensitivity", &Globals::fSensitivity, 0.1f, 5.0f);
    } 
    else if (Globals::activeTab == 1) { // MISC
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "[SAFE] BYPASSES");
        ToggleButton("Login Bypass (01 00 F0...)", &Globals::bBypass1);
        ToggleButton("Lobby Bypass (30 61...)", &Globals::bBypass2);
        ToggleButton("Anti-Cheat Bypass (58 01...)", &Globals::bBypass3);
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "[WARNING] EXPLOITS");
        ToggleButton("Speed Hack", &Globals::bSpeed);
        ToggleButton("Wallhack", &Globals::bWallhack);
        
        ImGui::Spacing();
        if (ImGui::Button("INJECT CHAMS (RED)", ImVec2(200, 30))) {
            // Logic to trigger Dllinjector
            InjectDLL("https://files.catbox.moe/3ps4f5.dll", "C:\\Windows\\Temp\\chams.dll");
        }
        
        ImGui::Spacing();
        if (ImGui::Button("SAFETY REVERT (ANTI-BAN)", ImVec2(560, 40))) {
            Globals::bRevert = true; // Logic in main loop
        }
    }
    else if (Globals::activeTab == 2) { // SETTINGS
        ImGui::Text("Info");
        ImGui::BulletText("Developer: REIMAN & BLOODIE");
        ImGui::BulletText("Status: Undetected (Optimized)");
        ImGui::Spacing();
        if (ImGui::Button("UNLOAD / EXIT", ImVec2(200, 30))) {
            exit(0);
        }
    }

    ImGui::EndChild();
    ImGui::End();
}
