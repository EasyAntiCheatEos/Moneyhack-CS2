#include "include/kdmapper.hpp"

#include <string>
#include <vector>
#include <filesystem>


HANDLE iqvw64e_device_handle;
bool debug = true;

bool sfc(_In_z_ _Printf_format_string_ const char* _Format, ...)
{
	if (debug == true)
	{
		va_list args;
		va_start(args, _Format);
		vprintf(_Format, args);  
		va_end(args);
	}

	return true; 
}

LONG WINAPI SimplestCrashHandler(EXCEPTION_POINTERS* ExceptionInfo)
{
	if (ExceptionInfo && ExceptionInfo->ExceptionRecord)
		sfc("crash\n");
	else
		sfc("crash\n");

	if (iqvw64e_device_handle)
		intel_driver::Unload(iqvw64e_device_handle);

	return EXCEPTION_EXECUTE_HANDLER;

}
bool callbackExample(ULONG64* param1, ULONG64* param2, ULONG64 allocationPtr, ULONG64 allocationSize, ULONG64 mdlptr) {
	UNREFERENCED_PARAMETER(param1);
	UNREFERENCED_PARAMETER(param2);
	UNREFERENCED_PARAMETER(allocationPtr);
	UNREFERENCED_PARAMETER(allocationSize);
	UNREFERENCED_PARAMETER(mdlptr);
	sfc("callback called\n");
	return true;
}
DWORD getParentProcess()
{
	HANDLE hSnapshot;
	PROCESSENTRY32 pe32;
	DWORD ppid = 0, pid = GetCurrentProcessId();

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	__try {
		if (hSnapshot == INVALID_HANDLE_VALUE) __leave;

		ZeroMemory(&pe32, sizeof(pe32));
		pe32.dwSize = sizeof(pe32);
		if (!Process32First(hSnapshot, &pe32)) __leave;

		do {
			if (pe32.th32ProcessID == pid) {
				ppid = pe32.th32ParentProcessID;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));

	}
	__finally {
		if (hSnapshot != INVALID_HANDLE_VALUE) CloseHandle(hSnapshot);
	}
	return ppid;
}
void PauseIfParentIsExplorer() {
	DWORD explorerPid = 0;
	GetWindowThreadProcessId(GetShellWindow(), &explorerPid);
	DWORD parentPid = getParentProcess();
	if (parentPid == explorerPid) {
		sfc("debugging\n");
		std::cin.get();
	}
}

void loaddrv(std::vector<uint8_t> driver)
{
	SetUnhandledExceptionFilter(SimplestCrashHandler);
	NTSTATUS exitCode = 0;
	kdmapper::AllocationMode mode = kdmapper::AllocationMode::AllocatePool;
	iqvw64e_device_handle = intel_driver::Load();
	if (!kdmapper::MapDriver(iqvw64e_device_handle, driver.data(), 0, 0, free, true, mode, false, callbackExample, &exitCode)) {
		intel_driver::Unload(iqvw64e_device_handle);
		PauseIfParentIsExplorer();
	}

	if (!intel_driver::Unload(iqvw64e_device_handle)) {
		sfc("fail unload\n");
		PauseIfParentIsExplorer();
	}
	sfc("load \n");
}