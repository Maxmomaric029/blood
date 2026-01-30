#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <TlHelp32.h>
#include <iostream>

class Memory {
public:
    HANDLE hProcess;
    DWORD processId;

    Memory(const std::string& processName);
    ~Memory();

    bool Attach(const std::string& processName);
    bool OpenProcessId(DWORD pid);
    void Close();
    
    // Hyper-Fast Scan logic
    uintptr_t FindPattern(const std::vector<unsigned char>& pattern);
    
    bool Patch(uintptr_t address, const std::vector<unsigned char>& bytes);
    bool WriteMemory(uintptr_t address, const std::vector<unsigned char>& bytes);
};
