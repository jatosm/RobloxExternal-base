#include "src/app/app.h"
#include "src/utility/logger/logger.h"
#include "src/utility/memory/syscall.h"
#include <Windows.h>
#include <shellapi.h>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
bool g_menuOpen = false;
bool g_vsync = false;
bool g_streamProof = false;
int g_fpsCap = 0;
float g_localSpeed = -1.0f;
namespace {
DWORD g_crashMainTid = 0;
bool is_admin() {
    BOOL admin = FALSE;
    SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
    PSID group = nullptr;
    if (AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &group)) {
        CheckTokenMembership(nullptr, group, &admin);
        FreeSid(group);
    }
    return admin != FALSE;
}
void relaunch_as_admin() {
    wchar_t exe[MAX_PATH]{};
    GetModuleFileNameW(nullptr, exe, MAX_PATH);
    SHELLEXECUTEINFOW sei{};
    sei.cbSize = sizeof(sei);
    sei.lpVerb = L"runas";
    sei.lpFile = exe;
    sei.nShow = SW_SHOWNORMAL;
    if (!ShellExecuteExW(&sei))
        thenwefuckin::app::badge("priv", thenwefuckin::app::C_RED, "elevation declined - run thenwefuckin.exe as administrator");
}
void on_terminate() {
    Logger::log("CRASH", "std::terminate (unhandled C++ exception?)");
    Logger::write_crash(nullptr, "terminate");
    MessageBoxA(nullptr, "thenwefuckin terminated unexpectedly.\nSee the .crash file on your Desktop.", "thenwefuckin crash", MB_ICONERROR);
    abort();
}
void __cdecl on_invalid_param(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t) {
    Logger::log("CRASH", "invalid CRT parameter");
    Logger::write_crash(nullptr, "invalid_parameter");
}
void __cdecl on_purecall() {
    Logger::log("CRASH", "pure virtual function call");
    Logger::write_crash(nullptr, "purecall");
    MessageBoxA(nullptr, "thenwefuckin crashed (purecall).\nSee the .crash file on your Desktop.", "thenwefuckin crash", MB_ICONERROR);
    _exit(3);
}
void on_sigabrt(int) {
    Logger::log("CRASH", "abort() called");
    Logger::write_crash(nullptr, "abort");
    _exit(3);
}
}
int main()
{
    using namespace thenwefuckin::app;
    g_crashMainTid = GetCurrentThreadId();
    SetUnhandledExceptionFilter(CrashHandler);
    AddVectoredExceptionHandler(1, [](PEXCEPTION_POINTERS ep)->LONG {
        if (IsDebuggerPresent()) return EXCEPTION_CONTINUE_SEARCH;
        DWORD code = ep->ExceptionRecord->ExceptionCode;
        if (code == 0x406D1388 || code == 0x40010006 || code == 0x4001000A || code == 0xE06D7363 ||
            code == 0x80000003 || code == 0x80000004 ||
            (code >= 0x000006A4 && code <= 0x000007CF)) return EXCEPTION_CONTINUE_SEARCH;

        if (ep && ep->ExceptionRecord && ep->ContextRecord && GetCurrentThreadId() != g_crashMainTid)
        {
            HMODULE self = GetModuleHandleW(nullptr);
            MODULEINFO mi{};
            if (self && GetModuleInformation(GetCurrentProcess(), self, &mi, sizeof(mi)))
            {
                auto base = (uintptr_t)mi.lpBaseOfDll;
                auto addr = (uintptr_t)ep->ExceptionRecord->ExceptionAddress;
                if (addr < base || addr >= base + (uintptr_t)mi.SizeOfImage)
                {
                    FARPROC exitThread = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "ExitThread");
                    if (exitThread)
                    {

                        Logger::write_crash(ep, "VEH-foreign", true);
                        ep->ContextRecord->Rip = (DWORD64)(uintptr_t)exitThread;
                        ep->ContextRecord->Rcx = 0;
                        return EXCEPTION_CONTINUE_EXECUTION;
                    }
                }
            }
        }
        static long long lastMs=0; static int count=0;
        long long now = GetTickCount64();
        if (now - lastMs < 500) return EXCEPTION_CONTINUE_SEARCH;
        if (count++ > 20) return EXCEPTION_CONTINUE_SEARCH;
        lastMs = now;
        Logger::write_crash(ep, "VEH");
        return EXCEPTION_CONTINUE_SEARCH;
    });
    std::set_terminate(on_terminate);
    _set_invalid_parameter_handler(on_invalid_param);
    _set_purecall_handler(on_purecall);
    signal(SIGABRT, on_sigabrt);
    _set_abort_behavior(0, _WRITE_ABORT_MSG);
    Logger::quiet = true;
    Logger::init();
    LOG("boot", "thenwefuckin starting...");
    LOG("info", ("log file: " + Logger::path).c_str());
    thenwefuckin::syscalls::init();
    LOG("boot", thenwefuckin::syscalls::ready()
        ? "memory path: direct syscalls (NtReadVirtualMemory/NtWriteVirtualMemory)"
        : "memory path: RPM/WPM fallback (syscall init failed)");
    if (!thenwefuckin::syscalls::ready())
        LOG("warn", "syscall init failed - using RPM/WPM fallback, expect slower reads");
    SetConsoleTitleA("thenwefuckin");
    std::cout << "\n";
    badge("thenwefuckin", C_CYAN, "external - INSERT to open menu");
    badge("discord", C_WHITE, "discord.gg/thenwefuckin");
    std::cout << "\n";
    HANDLE mutex = CreateMutexW(nullptr, FALSE, L"thenwefuckin-overlay-mutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        badge("fail", C_RED, "thenwefuckin is already running");
        LOG("error", "startup blocked: another instance already running (overlay mutex held)");
        MessageBoxW(nullptr, L"thenwefuckin is already running.", L"thenwefuckin", MB_OK | MB_ICONINFORMATION);
        if (mutex) CloseHandle(mutex);
        return 0;
    }
    LOG("info", "single-instance check passed");
    LOG("priv", "checking administrator privileges...");
    if (!is_admin()) {
        badge("priv", C_YELLOW, "administrator required - relaunching elevated...");
        LOG("warn", "not running as admin - relaunching elevated...");
        relaunch_as_admin();
        LOG("warn", "elevation declined or pending - run thenwefuckin.exe as administrator");
        if (mutex) CloseHandle(mutex);
        return 0;
    }
    badge("priv", C_GREEN, "running as administrator");
    LOG("ok", "running as administrator");
    badge("attaching", C_GREEN, "waiting for RobloxPlayerBeta.exe...");
    LOG("info", "waiting for RobloxPlayerBeta.exe...");
    LOG("boot", "session start");
    int rc = thenwefuckin::app::run();
    LOG("boot", ("session end rc=" + std::to_string(rc)).c_str());
    if (mutex) CloseHandle(mutex);
    return rc;
}
