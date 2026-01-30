#include <windows.h>
#include <dwmapi.h>
#include <d3d9.h>
#include <tchar.h>
#include <vector>
#include <iostream>
#include <string>
#include <sstream>
#include <thread>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "memory.h"
#include "gui.h"
#include "globals.h"
#include "hacks.h"

#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "d3d9.lib")

static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND hTargetWindow = NULL;
Memory* g_Mem = nullptr;

// --- CORE LOGIC ---
void HandleFeature(Feature& f, bool enabled) {
    if (!Globals::isAttached || !g_Mem) return;
    
    // Priority for Revert
    if (f.name == "Safety Revert" && enabled) {
        if (f.address != 0) g_Mem->Patch(f.address, f.replace); // Restore original bytes
        Globals::bRevert = false; // Reset toggle
        return;
    }

    if (enabled && !f.applied) {
        if (f.address == 0) f.address = g_Mem->FindPattern(f.search);
        
        if (f.address != 0) {
            if (g_Mem->Patch(f.address, f.replace)) {
                f.applied = true;
                std::cout << "[SUCCESS] Applied: " << f.name << std::endl;
            }
        }
    } else if (!enabled && f.applied) {
        if (f.address != 0 && g_Mem->Patch(f.address, f.search)) {
            f.applied = false;
            std::cout << "[RESTORED] " << f.name << std::endl;
        }
    }
}

void EmulatorThread() {
    while (true) {
        if (!Globals::isAttached) {
            if (g_Mem) delete g_Mem;
            g_Mem = new Memory("");
            
            for (const auto& emu : Globals::SUPPORTED_EMULATORS) {
                if (g_Mem->Attach(emu)) {
                    Globals::currentProcess = emu;
                    Globals::isAttached = true;
                    // Find window logic
                    if (emu == "HD-Player.exe") hTargetWindow = FindWindowA(NULL, "BlueStacks App Player");
                    if (!hTargetWindow) hTargetWindow = FindWindowA(NULL, "MSI App Player");
                    if (!hTargetWindow) hTargetWindow = FindWindowA(NULL, "LDPlayer");
                    break;
                }
            }
        } else {
            // Check if process died
            DWORD exitCode;
            if (GetExitCodeProcess(g_Mem->hProcess, &exitCode) && exitCode != STILL_ACTIVE) {
                Globals::isAttached = false;
                Globals::currentProcess = "None";
                hTargetWindow = NULL;
            }
        }
        Sleep(1000);
    }
}

int main(int, char**)
{
    AllocConsole();
    FILE* fp; freopen_s(&fp, "CONOUT$", "w", stdout);
    
    // Start background threads
    std::thread(EmulatorThread).detach();

    // UI Setup
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"BloodieOverlay", NULL };
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW, wc.lpszClassName, L"Bloodie Hack", WS_POPUP, 0, 0, 1920, 1080, NULL, NULL, wc.hInstance, NULL);

    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    if (!CreateDeviceD3D(hwnd)) { CleanupDeviceD3D(); return 1; }
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    SetupStyle();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        // Overlay Follow
        if (hTargetWindow && IsWindow(hTargetWindow)) {
            RECT rect; GetWindowRect(hTargetWindow, &rect);
            MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
        }

        // Logic
        if (Globals::isLoggedIn && Globals::isAttached) {
            HandleFeature(fBypass1, Globals::bBypass1);
            HandleFeature(fBypass2, Globals::bBypass2);
            HandleFeature(fBypass3, Globals::bBypass3);
            HandleFeature(fAimLock, Globals::bAimLock);
            HandleFeature(fSpeed, Globals::bSpeed);
            if (Globals::bRevert) HandleFeature(fRevert, true); // Trigger one-shot revert
        }

        // Render
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        // Fullscreen transparency for click-through
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("BG", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
        ImGui::End();

        if (Globals::bMenuVisible) RenderGUI();

        // Click-Through
        static bool wasVisible = true;
        if (Globals::bMenuVisible != wasVisible) {
            wasVisible = Globals::bMenuVisible;
            LONG_PTR style = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            if (Globals::bMenuVisible) style &= ~WS_EX_TRANSPARENT;
            else style |= WS_EX_TRANSPARENT;
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, style);
        }
        if (GetAsyncKeyState(VK_INSERT) & 1) Globals::bMenuVisible = !Globals::bMenuVisible;

        ImGui::EndFrame();
        g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCLEAR_ARGB(0, 0, 0, 0), 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        if (g_pd3dDevice->Present(NULL, NULL, NULL, NULL) == D3DERR_DEVICELOST) ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) return false;
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
    g_d3dpp.EnableAutoDepthStencil = TRUE;
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0) return false;
    return true;
}
void CleanupDeviceD3D() { if (g_pd3dDevice) g_pd3dDevice->Release(); if (g_pD3D) g_pD3D->Release(); }
void ResetDevice() { ImGui_ImplDX9_InvalidateDeviceObjects(); g_pd3dDevice->Reset(&g_d3dpp); ImGui_ImplDX9_CreateDeviceObjects(); }
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
        case WM_SIZE: if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) { g_d3dpp.BackBufferWidth = LOWORD(lParam); g_d3dpp.BackBufferHeight = HIWORD(lParam); ResetDevice(); } return 0;
        case WM_SYSCOMMAND: if ((wParam & 0xfff0) == SC_KEYMENU) return 0; break;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hWnd, msg, wParam, lParam);
}
