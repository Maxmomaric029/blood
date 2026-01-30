#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <vector>

namespace Globals {
    inline std::string currentProcess = "";
    inline bool isAttached = false;
    inline bool isLoggedIn = false;
    inline char licenseKey[256] = "";
    
    // UI State
    inline int activeTab = 0;
    inline bool bMenuVisible = true;
    
    // Features State (Buttons)
    inline bool bBypass1 = false;
    inline bool bBypass2 = false;
    inline bool bBypass3 = false;
    inline bool bRevert = false;
    
    inline bool bAimbotHead = false;
    inline bool bAimLock = false;
    inline bool bNoRecoil = false;
    
    inline bool bSpeed = false;
    inline bool bWallhack = false;
    inline bool bChams = false;
    inline bool bAntena = false;
    
    // Sliders
    inline float fFov = 90.0f;
    inline float fSensitivity = 1.0f;
    
    // Emulators
    const std::vector<std::string> SUPPORTED_EMULATORS = {
        "HD-Player.exe", "dnplayer.exe", "AndroidEmulatorEx.exe", "MuMuPlayer.exe", "MEmu.exe"
    };
}
