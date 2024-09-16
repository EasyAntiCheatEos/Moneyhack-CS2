#pragma once
#define WIN32_MEAN_AND_LEAN
#include <Windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <vector>
#include <iostream>
typedef struct _rw {
	INT32 security;
	INT32 process_id;
	ULONGLONG address;
	ULONGLONG buffer;
	ULONGLONG size;
	BOOLEAN write;
} rw, * prw;
typedef struct _ba {
	INT32 security;
	INT32 process_id;
	ULONGLONG* address;
} ba, * pba;
#define code_rw CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80001, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_ba CTL_CODE(FILE_DEVICE_UNKNOWN, 0x80002, METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define code_security 0x8457134f
class CDriver
{
private:

public:
	HANDLE Driver = 0x00;
	DWORD Id = 0x00;
	void write_physical(PVOID address, PVOID buffer, DWORD size);
	void read_physical(PVOID address, PVOID buffer, DWORD size);
	uintptr_t GetProcessBase();
	std::uint64_t GetModuleBase(const wchar_t* Name);
	bool loaded();
	template <typename T>
	T read(uint64_t address) {
		T buffer{ };
		read_physical((PVOID)address, &buffer, sizeof(T));
		return buffer;
	}
	template <typename T>
	T write(uint64_t address, T buffer) {

		write_physical((PVOID)address, &buffer, sizeof(T));
		return buffer;
	}
	bool IsValid(const uint64_t adress)
	{
		if (adress <= 0x400000 || adress == 0xCCCCCCCCCCCCCCCC || reinterpret_cast<void*>(adress) == nullptr || adress >
			0x7FFFFFFFFFFFFFFF) {
			return false;
		}
		return true;
	}
	template<typename T>
	bool ReadArray(uintptr_t address, T out[], size_t len)
	{
		for (size_t i = 0; i < len; ++i)
		{
			out[i] = read<T>(address + i * sizeof(T));
		}
		return true; // you can add additional checks to verify successful reads
	}
	template<typename T>
	bool ReadArray2(uint64_t address, T* out, size_t len)
	{
		if (!out || len == 0)
		{
			return false;
		}

		for (size_t i = 0; i < len; ++i)
		{
			if (!IsValid(address + i * sizeof(T)))
			{
				return false;
			}

			out[i] = read<T>(address + i * sizeof(T));
		}
		return true;
	}

	bool Attach(const wchar_t* Name);
};

inline CDriver* Driver = new CDriver();