#include "Auth/auth.hpp"
#include "Auth/utils.hpp"
#include "Auth/skStr.h"
#include "Auth/lazy.h"
#include "lib/lib.h"
#include "driver.h"
#include "../cheat/moneyhack.hpp"

using namespace KeyAuth;

std::string name = skCrypt("nurmolo").decrypt();
std::string ownerid = skCrypt("RR2z8bcHil").decrypt();
std::string secret = skCrypt("861eac1bd0fbbc506e0b6a8975474af2cea6af138cfacfa8794cc127a8cb1217").decrypt();
std::string version = skCrypt("1.0").decrypt();
std::string url = skCrypt("https://keyauth.win/api/1.2/").decrypt();
std::string path = skCrypt("").decrypt();

api AUTH(name, ownerid, secret, version, url, path);

namespace auth {
    std::string key;
    std::string subname;

    inline DWORD GetProcessIdByName(const std::wstring& processName) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (processName == pe32.szExeFile) {
                    CloseHandle(hSnapshot);
                    return pe32.th32ProcessID;
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
        return 0;
    }

    inline std::string getsubname() {
        if (AUTH.user_data.subscriptions.empty()) {
            return "";
        }
        return AUTH.user_data.subscriptions.front().name;
    }

    inline void terminateIfRunning(const std::wstring& processName) {
        DWORD pid = GetProcessIdByName(processName);
        if (pid > 0) {
            HANDLE processHandle = OpenProcess(PROCESS_TERMINATE, false, pid);
            TerminateProcess(processHandle, 0);
            CloseHandle(processHandle);
        }
    }

    inline void money() {
        while (true) {
            if (GetProcessIdByName(L"cs2.exe") == 0) {
                LI_FN(wprintf)(skCrypt(L"Please open Counter-Strike 2\n"));
                Sleep(2500);
            }
            else {
                moneyhack::entry();
            }
        }
    }

    inline void load() {
       // terminateIfRunning(L"money.exe");

        AllocConsole();
        name.clear();
        ownerid.clear();
        secret.clear();
        version.clear();
        url.clear();
        path.clear();

        LI_FN(SetConsoleTitleW)(skCrypt(L"moneyhacks"));
        LI_FN(wprintf)(skCrypt(L"Omg moneyhack $$$$$$$ 2024 release sigma kernel cs2\n"));
        LI_FN(wprintf)(skCrypt(L"\n\n\n\n\n"));
        LI_FN(wprintf)(skCrypt(L"Entry key -> "));
        std::cin >> key;

        AUTH.init();
        Sleep(100);
        AUTH.license(key);

        if (!AUTH.response.success) {
            LI_FN(wprintf)(skCrypt(L"Invalid key\n"));
            std::cin.get();
            exit(1);
        }

        subname = getsubname();
        if (subname != "moneyhackhvh") {
            exit(1);
        }

        if (!Driver->loaded()) {
            LI_FN(wprintf)(skCrypt(L"Mapping driver\n"));
            loaddrv(driverdat);
        }

        money();
    }
}
