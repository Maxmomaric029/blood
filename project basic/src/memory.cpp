#include "memory.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>

bool IsSameString(const std::string& a, const char* b) {
    std::string bStr(b);
    if (a.size() != bStr.size()) return false;
    return std::equal(a.begin(), a.end(), bStr.begin(),
                      [](unsigned char a, unsigned char b) {
                          return std::tolower(a) == std::tolower(b);
                      });
}

Memory::Memory(const std::string& processName) {
    this->hProcess = NULL;
    this->processId = 0;
}

Memory::~Memory() {
    Close();
}

void Memory::Close() {
    if (this->hProcess) {
        CloseHandle(this->hProcess);
        this->hProcess = NULL;
    }
}

bool Memory::Attach(const std::string& processName) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe; 
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe)) {
        do {
#ifdef UNICODE
            std::wstring wStr(pe.szExeFile);
            std::string sExeFile(wStr.begin(), wStr.end());
#else
            std::string sExeFile(pe.szExeFile);
#endif
            if (IsSameString(processName, sExeFile.c_str())) {
                this->processId = pe.th32ProcessID;
                this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, this->processId);
                CloseHandle(hSnap);
                return (this->hProcess != NULL);
            }
        } while (Process32Next(hSnap, &pe));
    }
    CloseHandle(hSnap);
    return false;
}

bool Memory::OpenProcessId(DWORD pid) {
    this->processId = pid;
    this->hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    return (this->hProcess != NULL);
}

// Optimized scanner: Reads entire regions
uintptr_t Memory::FindPattern(const std::vector<unsigned char>& pattern) {
    if (!this->hProcess || pattern.empty()) return 0;

    unsigned char* pRemote = nullptr;
    MEMORY_BASIC_INFORMATION mbi;

    // We use a large buffer to read chunks. 
    // Emulators store data in MEM_PRIVATE/MEM_MAPPED chunks.
    while (VirtualQueryEx(this->hProcess, pRemote, &mbi, sizeof(mbi))) {
        // Optimization: Only scan ReadWrite or ExecuteReadWrite memory.
        bool isReadWrite = (mbi.Protect & (PAGE_READWRITE | PAGE_EXECUTE_READWRITE));
        bool isCommit = (mbi.State == MEM_COMMIT);
        
        if (isCommit && isReadWrite && (mbi.Type == MEM_PRIVATE || mbi.Type == MEM_MAPPED)) {
            // Read the whole region
            std::vector<unsigned char> buffer(mbi.RegionSize);
            SIZE_T bytesRead;
            if (ReadProcessMemory(this->hProcess, mbi.BaseAddress, buffer.data(), mbi.RegionSize, &bytesRead)) {
                
                // std::search is efficient enough for linear scan in a buffer
                auto it = std::search(buffer.begin(), buffer.end(), pattern.begin(), pattern.end(),
                    [](unsigned char a, unsigned char b) {
                        return (b == 0x00) || (a == b); // 0x00 as wildcard in vector if manually set, mostly direct match
                    });

                if (it != buffer.end()) {
                    size_t offset = std::distance(buffer.begin(), it);
                    uintptr_t foundAddr = (uintptr_t)mbi.BaseAddress + offset;
                    
                    // ARM Alignment Check (4 bytes)
                    if (foundAddr % 4 == 0) {
                        return foundAddr;
                    }
                }
            }
        }
        pRemote += mbi.RegionSize;
    }
    return 0;
}

bool Memory::WriteMemory(uintptr_t address, const std::vector<unsigned char>& bytes) {
    if (!this->hProcess || address == 0) return false;
    
    DWORD oldProtect;
    if (!VirtualProtectEx(this->hProcess, (LPVOID)address, bytes.size(), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        VirtualProtectEx(this->hProcess, (LPVOID)address, bytes.size(), PAGE_READWRITE, &oldProtect);
    }

    SIZE_T written;
    bool success = WriteProcessMemory(this->hProcess, (LPVOID)address, bytes.data(), bytes.size(), &written);

    VirtualProtectEx(this->hProcess, (LPVOID)address, bytes.size(), oldProtect, &oldProtect);
    FlushInstructionCache(this->hProcess, (LPCVOID)address, bytes.size());
    return success;
}

bool Memory::Patch(uintptr_t address, const std::vector<unsigned char>& bytes) {
    return WriteMemory(address, bytes);
}
