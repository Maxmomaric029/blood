#pragma once
#include <vector>
#include <string>
#include <sstream>

// Helper to parse hex strings
inline std::vector<unsigned char> ParseHex(const std::string& hex) {
    std::vector<unsigned char> bytes;
    std::stringstream ss(hex);
    std::string byteStr;
    while (ss >> byteStr) {
        if (byteStr == "?" || byteStr == "??") bytes.push_back(0x00);
        else {
            try { bytes.push_back((unsigned char)std::stoi(byteStr, nullptr, 16)); }
            catch (...) { bytes.push_back(0x00); }
        }
    }
    return bytes;
}

struct Feature {
    std::string name;
    std::vector<unsigned char> search;
    std::vector<unsigned char> replace;
    uintptr_t address = 0;
    bool applied = false;
    int riskLevel; // 0=Safe, 1=Risk, 2=Ban
};

// --- BYPASSES (Priority) ---
inline Feature fBypass1 = { "Login Bypass", 
    ParseHex("01 00 A0 E3 1C 00 86 E5 00 70 94 E5 C0 60 95 E5"), 
    ParseHex("01 00 F0 E3 1C 00 86 E5 00 70 94 E5 C0 60 95 E5"), 
    0, false, 0 
};

inline Feature fBypass2 = { "Lobby Bypass", 
    ParseHex("38 61 87 E5 00 70 94 E5 CC 60 95 E5 00 00 57 E3"), 
    ParseHex("30 61 87 E5 00 70 94 E5 CC 60 95 E5 00 00 57 E3"), 
    0, false, 0 
};

inline Feature fBypass3 = { "Anti-Cheat Bypass", 
    ParseHex("54 01 87 E5 00 70 94 E5 01 10 9F E7 00 00 91 E5"), 
    ParseHex("58 01 87 E5 00 70 94 E5 01 10 9F E7 00 00 91 E5"), 
    0, false, 0 
};

inline Feature fRevert = { "Safety Revert",
    ParseHex("30 48 2D E9 08 B0 8D E2 00 40 A0 E1 84 00 04 E3 00 10 A0 E3 E6 C4 E3 EB 01 00 50 E3 28 00 94 15 30 88 BD 18 84 00 04 E3"),
    ParseHex("00 00 A0 E3 1E FF 2F E1 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"), 
    0, false, 0
};

// --- AIMBOT ---
inline Feature fAimLock = { "Aim Lock", 
    ParseHex("C0 3F 0A D7 A3 3B 0A D7 A3 3B 8F C2 75 3D AE 47 E1 3D 9A 99 19 3E CD CC 4C 3E A4 70 FD 3E"), 
    ParseHex("90 65 0A D7 A3 3B 0A D7 A3 3B 8F C2 75 3D AE 47 E1 3D 9A 99 19 3E CD CC 4C 3E A4 70 FD 3E"), 
    0, false, 1 
};

inline Feature fAimbotHead = { "Aim Head OB52",
    ParseHex("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3F 00 00 80 3F 00 00 80 3F 00 00 80 3F 00 00 80 3F F5 F4 74 3E"),
    ParseHex("00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 80 3F 00 00 80 3F 00 00 80 3F 00 00 80 3F F5 F4 74 3E"),
    0, false, 1
};

// --- MISC ---
inline Feature fSpeed = { "Speed Hack", 
    ParseHex("02 2B 07 3D 02 2B 07 3D 02 2B 07 3D 00 00 00 00 9B 6C"), 
    ParseHex("E3 A5 9B 3C E3 A5 9B 3C 02 2B 07 3D 00 00 00 00 9B 6C"), 
    0, false, 2 
};

inline Feature fWallhack = { "Wallhack",
    ParseHex("00 00 00 00 00 AE 47 81 3F AE 47 81 3F AE 47 81 3F AE 47 81 3F 00 1A B7 EE DC 3A 9F ED 30 00 4F E2 43 2A B0 EE EF 0A 60 F4 43 6A F0 EE 1C 00 8A E2 43 5A F0 EE 8F 0A 48 F4 43 2A F0 EE 43 7A B0 EE 8F 0A 40 F4 41 AA B0 EE FE"),
    ParseHex("00 00 00 00 00 AE 47 81 3F AE 47 81 3F AE 47 81 BF AE 47 81 3F AE 47 81 BF AE 47 81 3F 00 1A B7 EE DC 3A 9F ED EF 0A 60 F4 43 6A F0 EE 1C 00 8A E2 43 5A F0 EE 8F 0A 48 F4 43 2A F0 EE 43 7A B0 EE 8F 0A 40 F4 41 AA B0 EE FE"),
    0, false, 2
};