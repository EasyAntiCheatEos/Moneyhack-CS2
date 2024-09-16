#pragma once
#pragma warning(disable:4996)
#include "moneyhack.hpp"


inline namespace storage
{
	inline uintptr_t B_CLIENTDLL;
	inline uintptr_t B_ENGINE2DLL;
	inline uintptr_t B_SCENESYSTEMDLL;
	inline static ULONG DISPLAYWIDTH = GetSystemMetrics(SM_CXSCREEN);
	inline static ULONG DISPLAYHEIGHT = GetSystemMetrics(SM_CYSCREEN);
	inline HWND cs2window;
	inline HWND activewindow;
	inline static int Width = 0;
	inline static int Height = 0;
	inline int screenCenterX = storage::DISPLAYWIDTH / 2;
	inline int screenCenterY = storage::DISPLAYHEIGHT / 2;
	ImVec2 center(storage::DISPLAYWIDTH / 2, storage::DISPLAYHEIGHT / 2);
}

inline namespace CONFIG
{
	struct CHEATCONFIG
	{
		// visual
		bool box3d;

		bool box2d;
		int box2dtype;

		bool SkellyBones;

		bool SnapLines;
		int SnapLineType;

		bool healthesp;
		bool nameesp;

		bool fovcircle_B;
		float fovcircle_S = 150.f;
		
		int box3dthickness = 3.f;
		int skellbonesthickness = 2.f;
		int box2dthickness = 3.f;
		int snaplinethickness = 1;
		// colors
		float box2d_C[4] = { 1.0f, 0.0f, 0.0f, 0.7f };
		float box3d_C[4] = { 0.0f, 0.0f, 0.0f, 0.70f };
		float snapline_C[4] = { 0.0f, 0.0f, 0.0f, 0.70f };
		float SkellyBones_C[4] = { 155.0f, 155.0f, 155.0f, 0.7f };

	} cfg;
}
namespace offsets
{
	inline DWORD64 dwEntityList = 0x1954568;
	inline DWORD64 dwLocalPlayerPawn = 0x17C17F0;
	inline DWORD64 dwViewMatrix = 0x19B64F0;
	inline DWORD64 m_hPlayerPawn = 0x7EC;
	namespace client_dll
	{
		inline DWORD64 m_iTeamNum = 0x3C3;
		inline DWORD64 m_iHealth = 0x324;
		inline DWORD64 m_vOldOrigin = 0x1274;
		inline DWORD64 m_sSanitizedPlayerName = 0x750; // CUtlString
		inline DWORD64 m_steamID = 0x6C8; // uint64
	}
}