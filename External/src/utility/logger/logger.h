#pragma once
#include <string>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <vector>
#include <windows.h>
#include <psapi.h>
#include <dbghelp.h>
#include "../struct/offsets.h"
#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "psapi.lib")
namespace Logger {
    constexpr WORD LW = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    constexpr WORD LD = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    constexpr WORD LR = FOREGROUND_RED | FOREGROUND_INTENSITY;
    constexpr WORD LY = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    constexpr WORD LG = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    constexpr WORD LC = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    constexpr WORD LM = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    inline std::string lower_of(const std::string& s) {
        std::string o = s;
        for (char& c : o) c = (char)tolower((unsigned char)c);
        return o;
    }
    inline std::string upper_of(const std::string& s) {
        std::string o = s;
        for (char& c : o) c = (char)toupper((unsigned char)c);
        return o;
    }
    inline WORD level_color(const std::string& tag) {
        std::string t = lower_of(tag);
        if (t == "crash" || t == "error" || t == "fail" || t == "fatal" || t == "exception" ||
            t == "abort" || t == "terminate" || t == "purecall" || t == "invalid_parameter")
            return LR;
        if (t == "warn" || t == "warning" || t == "left" || t == "priv" || t == "fallback")
            return LY;
        if (t == "ok" || t == "success" || t == "ready" || t == "attached" || t == "done" ||
            t == "game" || t == "joined")
            return LG;
        if (t == "boot" || t == "attach" || t == "attaching" || t == "overlay" || t == "syscall" ||
            t == "session" || t == "info" || t == "init")
            return LC;
        if (t == "world" || t == "fly" || t == "explorer" || t == "aim" || t == "visual" || t == "misc")
            return LM;
        if (t == "debug" || t == "trace" || t == "verbose")
            return LD;
        return LW;
    }
    inline bool is_debug_tag(const std::string& tag) {
        std::string t = lower_of(tag);
        return t == "debug" || t == "trace" || t == "verbose";
    }
    inline void print_console(const std::string& ts, const std::string& tag, const std::string& msg) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        if (!h || h == INVALID_HANDLE_VALUE) {
            std::cout << "[" << ts << "] - [" << upper_of(tag) << "] " << msg << std::endl;
            return;
        }
        CONSOLE_SCREEN_BUFFER_INFO old{};
        GetConsoleScreenBufferInfo(h, &old);
        WORD lc = level_color(tag);
        WORD msgColor = (lc == LR || lc == LY) ? LW : LD;
        if (is_debug_tag(tag)) msgColor = LD;
        SetConsoleTextAttribute(h, LW); std::cout << "[";
        SetConsoleTextAttribute(h, LR); std::cout << ts;
        SetConsoleTextAttribute(h, LW); std::cout << "]";
        SetConsoleTextAttribute(h, LD); std::cout << " - ";
        SetConsoleTextAttribute(h, LW); std::cout << "[";
        SetConsoleTextAttribute(h, lc); std::cout << upper_of(tag);
        SetConsoleTextAttribute(h, LW); std::cout << "] ";
        SetConsoleTextAttribute(h, msgColor); std::cout << msg << "\n";
        SetConsoleTextAttribute(h, old.wAttributes);
    }
    inline std::mutex mtx;
    inline std::atomic<bool> crashActive{false};
    inline std::ofstream file;
    inline std::string path;
    inline std::string crashPath;
    inline std::string session = "no game yet";
    inline bool initialized = false;
    inline bool crashWritten = false;
    inline bool quiet = false;
    inline std::vector<std::string> recent;
    inline std::string now_str() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto t = system_clock::to_time_t(now);
        std::tm tm{}; localtime_s(&tm, &t);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms.count();
        return oss.str();
    }
    inline std::string stamp_file() {
        auto now = std::chrono::system_clock::now();
        auto t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        std::tm tm{}; localtime_s(&tm, &t);
        char b[48]; strftime(b,sizeof(b),"%Y%m%d_%H%M%S",&tm);
        char out[64]; snprintf(out,sizeof(out),"%s_%03d", b, (int)ms.count());
        return out;
    }
    inline void note_session(const std::string& s) {
        std::lock_guard<std::mutex> lk(mtx);
        session = s;
    }
    inline void prune_old_crashes(const std::string& desk) {
        std::string pattern = desk + "\\thenwefuckin_*.crash";
        WIN32_FIND_DATAA fd{};
        HANDLE h = FindFirstFileA(pattern.c_str(), &fd);
        if (h == INVALID_HANDLE_VALUE) return;
        std::vector<std::string> names;
        do { names.emplace_back(fd.cFileName); } while (FindNextFileA(h, &fd));
        FindClose(h);
        if (names.size() <= 10) return;
        std::sort(names.begin(), names.end());
        for (size_t i = 0; i + 10 < names.size(); i++)
            DeleteFileA((desk + "\\" + names[i]).c_str());
    }
    inline void init() {
        std::lock_guard<std::mutex> lk(mtx);
        if (initialized) return;
        char* up = nullptr; size_t len = 0;
        _dupenv_s(&up, &len, "USERPROFILE");
        std::string desk = up ? std::string(up) : "C:\\Users\\Public";
        if (up) free(up);
        path = desk + "\\Desktop\\thenwefuckin_log.txt";
        file.open(path, std::ios::out | std::ios::app);
        initialized = true;
        if (file.is_open()) file << "\n========== thenwefuckin log start " << now_str() << " ==========\n" << std::flush;
        prune_old_crashes(desk + "\\Desktop");
    }
    inline void log(const std::string& tag, const std::string& msg) {
        if (!initialized) init();
        std::string ts = now_str();
        std::string line = "[" + ts + "] - [" + tag + "] " + msg;
        {
            std::lock_guard<std::mutex> lk(mtx);
            recent.push_back(line);
            if (recent.size() > 500) recent.erase(recent.begin());
            if (!quiet || !is_debug_tag(tag)) print_console(ts, tag, msg);
            if (file.is_open()) { file << line << "\n" << std::flush; }
        }
        OutputDebugStringA((line + "\n").c_str());
    }
    inline void info(const std::string& msg) { log("INFO", msg); }
    inline void ok(const std::string& msg) { log("OK", msg); }
    inline void warn(const std::string& msg) { log("WARN", msg); }
    inline void error(const std::string& msg) { log("ERROR", msg); }
    inline void debug(const std::string& msg) { log("DEBUG", msg); }
    inline void logf(const std::string& tag, const char* fmt, ...) {
        char buf[2048];
        va_list ap; va_start(ap, fmt); vsnprintf(buf, sizeof(buf), fmt, ap); va_end(ap);
        log(tag, buf);
    }

    inline void write_crash(EXCEPTION_POINTERS* ep, const char* source, bool force = false) {
        if (!force && crashActive.exchange(true)) return;
        std::unique_lock<std::mutex> lk(mtx, std::defer_lock);
        if (force)
        {

            if (!lk.try_lock()) return;
        }
        else
        {
            lk.lock();
        }
        char* up = nullptr; size_t len = 0;
        _dupenv_s(&up, &len, "USERPROFILE");
        std::string desk = up ? std::string(up) : "C:\\Users\\Public";
        if (up) free(up);
        std::string tag;
        if (source && std::string(source).find("foreign") != std::string::npos)
            tag = "_foreign";
        crashPath = desk + "\\Desktop\\thenwefuckin_" + stamp_file() + tag + ".crash";
        char exePath[MAX_PATH]={0};
        GetModuleFileNameA(nullptr, exePath, MAX_PATH);
        std::string exeDir = exePath;
        auto slash = exeDir.find_last_of("\\/");
        if (slash != std::string::npos) exeDir = exeDir.substr(0, slash);
        std::string buildCrash = exeDir + "\\crash.crash";
        std::ofstream cf(crashPath, std::ios::out);
        std::ofstream bf(buildCrash, std::ios::out);
        if (!cf.is_open() && !bf.is_open()) return;
        auto write_all = [&](std::ofstream& out){
            if (!out.is_open()) return;
            out << "========== THENWEFUCKIN CRASH REPORT ==========\n";
            out << "time: " << now_str() << " source: " << source << "\n";
            out << "session: " << session << "\n";
            out << "client: " << Offsets::ClientVersion << "\n";
            if (ep && ep->ExceptionRecord) {
                out << "exception code: 0x" << std::hex << ep->ExceptionRecord->ExceptionCode << std::dec << "\n";
                out << "exception addr: 0x" << std::hex << (uintptr_t)ep->ExceptionRecord->ExceptionAddress << std::dec << "\n";
                out << "exception flags: 0x" << std::hex << ep->ExceptionRecord->ExceptionFlags << std::dec << "\n";
                out << "params: " << ep->ExceptionRecord->NumberParameters;
                for (DWORD i=0;i<ep->ExceptionRecord->NumberParameters;i++) out << " 0x" << std::hex << ep->ExceptionRecord->ExceptionInformation[i] << std::dec;
                out << "\n";
            }
            {

                std::string faultMod = "?";
                if (ep && ep->ExceptionRecord)
                {
                    auto addr = (uintptr_t)ep->ExceptionRecord->ExceptionAddress;
                    HMODULE mods[256]; DWORD need = 0;
                    HANDLE proc = GetCurrentProcess();
                    if (EnumProcessModules(proc, mods, sizeof(mods), &need))
                    {
                        for (size_t i = 0; i < need / sizeof(HMODULE); i++)
                        {
                            MODULEINFO mi{}; GetModuleInformation(proc, mods[i], &mi, sizeof(mi));
                            auto base = (uintptr_t)mi.lpBaseOfDll;
                            if (addr >= base && addr < base + mi.SizeOfImage)
                            {
                                char name[MAX_PATH] = {0}; GetModuleFileNameA(mods[i], name, MAX_PATH);
                                faultMod = name;
                                break;
                            }
                        }
                    }
                }
                out << "fault module: " << faultMod << "\n";
            }
            if (ep && ep->ContextRecord) {
#ifdef _M_X64
                auto c = ep->ContextRecord;
                out << "\n--- registers (x64) ---\n";
                out << "RAX=" << std::hex << c->Rax << " RBX=" << c->Rbx << " RCX=" << c->Rcx << " RDX=" << c->Rdx << "\n";
                out << "RSI=" << c->Rsi << " RDI=" << c->Rdi << " RBP=" << c->Rbp << " RSP=" << c->Rsp << "\n";
                out << "R8 =" << c->R8 << " R9 =" << c->R9 << " R10=" << c->R10 << " R11=" << c->R11 << "\n";
                out << "R12=" << c->R12 << " R13=" << c->R13 << " R14=" << c->R14 << " R15=" << c->R15 << "\n";
                out << "RIP=" << c->Rip << " EFL=" << c->EFlags << "\n";
#endif
            }
            out << "\n--- callstack ---\n";
            {
                void* st[64]; USHORT n = CaptureStackBackTrace(0, 64, st, nullptr);
                HANDLE proc = GetCurrentProcess();
                SymInitialize(proc, nullptr, TRUE);
                for (USHORT i=0;i<n;i++) {
                    DWORD64 addr = (DWORD64)st[i];
                    char symBuf[sizeof(SYMBOL_INFO)+256]; SYMBOL_INFO* sym=(SYMBOL_INFO*)symBuf;
                    sym->SizeOfStruct=sizeof(SYMBOL_INFO); sym->MaxNameLen=255;
                    DWORD64 disp=0;
                    if (SymFromAddr(proc, addr, &disp, sym)) out << "#" << i << " 0x" << std::hex << addr << std::dec << " " << sym->Name << "+0x" << std::hex << disp << std::dec << "\n";
                    else out << "#" << i << " 0x" << std::hex << addr << std::dec << "\n";
                }
            }
            out << "\n--- recent log (" << recent.size() << " lines) ---\n";
            for (auto &l: recent) out << l << "\n";
            out << "\n--- modules ---\n";
            {
                HMODULE mods[256]; DWORD need=0;
                HANDLE proc = GetCurrentProcess();
                if (EnumProcessModules(proc, mods, sizeof(mods), &need)) {
                    for (size_t i=0;i<need/sizeof(HMODULE);i++) {
                        char name[MAX_PATH]={0}; GetModuleFileNameA(mods[i], name, MAX_PATH);
                        MODULEINFO mi{}; GetModuleInformation(proc, mods[i], &mi, sizeof(mi));
                        out << std::hex << (uintptr_t)mi.lpBaseOfDll << "-" << (uintptr_t)mi.lpBaseOfDll+mi.SizeOfImage << std::dec << " " << name << "\n";
                    }
                }
            }
            out << "\n--- system ---\n";
#pragma warning(push)
#pragma warning(disable:4996)
            { OSVERSIONINFOA vi{}; vi.dwOSVersionInfoSize=sizeof(vi); GetVersionExA(&vi); out << "windows " << vi.dwMajorVersion << "." << vi.dwMinorVersion << " build " << vi.dwBuildNumber << "\n"; }
#pragma warning(pop)
            out << "\n========== END ==========\n";
        };
        write_all(cf); write_all(bf);
        cf.flush(); bf.flush();
        crashWritten = true;
        if (file.is_open()) { file << "\n!!! CRASH written to " << crashPath << " + " << buildCrash << " !!!\n" << std::flush; }
        std::string cmsg = "CRASH written to " + crashPath + " + " + buildCrash;
        print_console(now_str(), "CRASH", cmsg);
    }
}
#define LOG(tag, msg) Logger::log(tag, msg)
#define LOGF(tag, fmt, ...) Logger::logf(tag, fmt, __VA_ARGS__)
inline LONG WINAPI CrashHandler(EXCEPTION_POINTERS* ep) {
    if (IsDebuggerPresent()) return EXCEPTION_CONTINUE_SEARCH;

    if (ep && ep->ExceptionRecord)
    {
        HMODULE self = GetModuleHandleW(nullptr);
        MODULEINFO mi{};
        if (self && GetModuleInformation(GetCurrentProcess(), self, &mi, sizeof(mi)))
        {
            auto base = (uintptr_t)mi.lpBaseOfDll;
            auto addr = (uintptr_t)ep->ExceptionRecord->ExceptionAddress;
            if (addr < base || addr >= base + (uintptr_t)mi.SizeOfImage)
            {

                Logger::write_crash(ep, "UEF-foreign", true);
                if (Logger::file.is_open()) Logger::file.flush();
                return EXCEPTION_CONTINUE_SEARCH;
            }
        }
    }
    Logger::log("CRASH", "unhandled exception!");
    if (ep && ep->ExceptionRecord) {
        char buf[256]; snprintf(buf, sizeof(buf), "code=0x%08X addr=0x%p", (unsigned)ep->ExceptionRecord->ExceptionCode, ep->ExceptionRecord->ExceptionAddress);
        Logger::log("CRASH", buf);
    }
    Logger::write_crash(ep, "UEF");
    if (Logger::file.is_open()) Logger::file.flush();
    std::string msg = "thenwefuckin crashed!\n\nCrash report:\n" + Logger::crashPath + "\n\nLog:\n" + Logger::path;
    MessageBoxA(nullptr, msg.c_str(), "thenwefuckin crash", MB_ICONERROR);
    return EXCEPTION_EXECUTE_HANDLER;
}
