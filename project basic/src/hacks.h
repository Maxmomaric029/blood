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
    ParseHex("00 00 A0 E3 1E FF 2F E1 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00"), // Padding 0s to match length if needed, or exact replacement
    0, false, 0
};
// Note for Revert: The user said "Rep: 00 00 A0 E3 1E FF 2F E1" which is shorter. 
// The patcher will handle partial write or we pad it. Ideally we write only the bytes provided.

// --- AIMBOT ---
inline Feature fAimLock = { "Aim Lock", 
    ParseHex("C0 3F 0A D7 A3 3B 0A D7 A3 3B 8F C2 75 3D AE 47 E1 3D 9A 99 19 3E CD CC 4C 3E A4 70 FD 3E"), 
    ParseHex("90 65 0A D7 A3 3B 0A D7 A3 3B 8F C2 75 3D AE 47 E1 3D 9A 99 19 3E CD CC 4C 3E A4 70 FD 3E"), 
    0, false, 1 
};

// --- MISC ---
inline Feature fSpeed = { "Speed Hack", 
    ParseHex("02 2B 07 3D 02 2B 07 3D 02 2B 07 3D 00 00 00 00 9B 6C"), 
    ParseHex("E3 A5 9B 3C E3 A5 9B 3C 02 2B 07 3D 00 00 00 00 9B 6C"), 
    0, false, 2 
};
