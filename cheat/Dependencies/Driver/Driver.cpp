#include "Driver.h"
#include <cwchar> // Include this header for wcscmp
#include <TlHelp32.h>
#include "../../xor.hpp"
void CDriver::read_physical(PVOID address, PVOID buffer, DWORD size) {
    _rw arguments = { 0 };

    arguments.security = code_security;
    arguments.address = (ULONGLONG)address;
    arguments.buffer = (ULONGLONG)buffer;
    arguments.size = size;
    arguments.process_id = this->Id;
    arguments.write = FALSE;

    DeviceIoControl(this->Driver, code_rw, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}
void CDriver::write_physical(PVOID address, PVOID buffer, DWORD size) {
    _rw arguments = { 0 };

    arguments.security = code_security;
    arguments.address = (ULONGLONG)address;
    arguments.buffer = (ULONGLONG)buffer;
    arguments.size = size;
    arguments.process_id = this->Id;
    arguments.write = TRUE;

    DeviceIoControl(this->Driver, code_rw, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);
}
uintptr_t CDriver::GetProcessBase() {
    uintptr_t image_address = { NULL };
    _ba arguments = { NULL };

    arguments.security = code_security;
    arguments.process_id = this->Id;
    arguments.address = (ULONGLONG*)&image_address;

    DeviceIoControl(this->Driver, code_ba, &arguments, sizeof(arguments), nullptr, NULL, NULL, NULL);

    return image_address;
}
std::uint64_t CDriver::GetModuleBase(const wchar_t* Name)
{
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, this->Id);

    if (Snapshot == INVALID_HANDLE_VALUE)
    {
        return 0x00;
    }

    MODULEENTRY32W Entry; // Note the 'W' suffix indicating the wide-character version
    Entry.dwSize = sizeof(Entry);

    if (Module32FirstW(Snapshot, &Entry)) // Use Module32FirstW for wide-character support
    {
        do
        {
            if (!wcscmp(Entry.szModule, Name))
            {
                CloseHandle(Snapshot);
                return reinterpret_cast<std::uint64_t>(Entry.modBaseAddr);
            }
        } while (Module32NextW(Snapshot, &Entry)); // Use Module32NextW for wide-character support
    }

    CloseHandle(Snapshot);
    return 0x00;
}
bool CDriver::Attach(const wchar_t* Name)
{
    DWORD processId = 0;
    PROCESSENTRY32W processEntry;
    processEntry.dwSize = sizeof(PROCESSENTRY32W);

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return 0; // Snapshot failed
    }

    if (Process32FirstW(snapshot, &processEntry))
    {
        do
        {
            if (!_wcsicmp(processEntry.szExeFile, Name)) // Case-insensitive comparison
            {
                processId = processEntry.th32ProcessID;
                this->Id = processId;
                break;
            }
        } while (Process32NextW(snapshot, &processEntry));
    }

    CloseHandle(snapshot);

    this->Driver = CreateFileW(xorstr_(L"\\\\.\\moneyhackhvh"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (!this->Driver || (this->Driver == INVALID_HANDLE_VALUE))
        return false;

    return true;
}

bool CDriver::loaded() {
    HANDLE temp = CreateFileW(xorstr_(L"\\\\.\\moneyhackhvh"), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

    if (temp == INVALID_HANDLE_VALUE) {
        return false;
    }

    CloseHandle(temp);
    return true;
}