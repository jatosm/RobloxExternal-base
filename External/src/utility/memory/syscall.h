#pragma once
#include <Windows.h>
#include <cstdint>
extern "C" long Rax_ReadVirtualMemory(void* processHandle, const void* baseAddress,
    void* buffer, unsigned long long numberOfBytesToRead, unsigned long long* numberOfBytesRead);
extern "C" long Rax_WriteVirtualMemory(void* processHandle, void* baseAddress,
    const void* buffer, unsigned long long numberOfBytesToWrite, unsigned long long* numberOfBytesWritten);
extern "C" void* Rax_MemCpy(void* dst, const void* src, unsigned long long size);
extern "C" void* Rax_MemSet(void* dst, unsigned char value, unsigned long long size);
#ifdef _M_X64
extern "C" void Rax_SetSsns(unsigned long readSsn, unsigned long writeSsn);
#endif
namespace thenwefuckin::syscalls
{
#ifdef _M_X64
    inline unsigned long ntdll_ssn(const char* name)
    {
        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (!ntdll) return 0;
        auto fn = reinterpret_cast<const unsigned char*>(GetProcAddress(ntdll, name));
        if (!fn) return 0;
        if (fn[0] != 0x4C || fn[1] != 0x8B || fn[2] != 0xD1 || fn[3] != 0xB8)
            return 0;
        unsigned long ssn = static_cast<unsigned long>(fn[4]) | (static_cast<unsigned long>(fn[5]) << 8);
        if (ssn == 0 || ssn > 0xFFFF) return 0;
        return ssn;
    }
    inline bool& ready_flag()
    {
        static bool v = false;
        return v;
    }
    inline void init()
    {
        static bool done = false;
        if (done) return;
        done = true;
        unsigned long r = ntdll_ssn("NtReadVirtualMemory");
        unsigned long w = ntdll_ssn("NtWriteVirtualMemory");
        if (r != 0 && w != 0)
        {
            Rax_SetSsns(r, w);
            ready_flag() = true;
        }
    }
    inline bool ready()
    {
        return ready_flag();
    }
#else
    inline void init() {}
    inline bool ready() { return false; }
#endif
}
