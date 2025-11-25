// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "..\1) IATHooker\IATHooker.h"
#include <fstream>

decltype(::GetSysColor)* GetSysColorOrg;

void LogIndex(int index)
{
	const wchar_t* path =
		L"D:\\GetSysColorLog.txt";

	std::wofstream file(path, std::ios::app);
	file << "GetSysColor(" << index << ")\n";
}


COLORREF WINAPI GetSysColorHooked(int index) {

	LogIndex(index);

	switch (index) {
	case COLOR_BTNTEXT:
		return RGB(0, 128, 0);

	case COLOR_WINDOWTEXT:
		return RGB(0, 0, 255);

	case COLOR_WINDOW:
		return RGB(255, 0, 0);

	case COLOR_MENU:
		return RGB(255, 0, 0);
	}

	/*wchar_t buf1[128];
	swprintf(buf1, 128, L"Index %d", index);
	MessageBox(nullptr, buf1, L"Debug", MB_OK);*/

	return GetSysColorOrg(index);
	// return ((decltype(::GetSysColor)*)::GetProcAddress(GetModuleHandle(L"user32"), "GetSysColor"))(index);
}


void HookFunctions() {
	auto hUser32 = ::GetModuleHandle(L"user32.dll");
	// save original functions
	if (hUser32)
		GetSysColorOrg = (decltype(GetSysColorOrg))::GetProcAddress(hUser32, "GetSysColor");

	auto count = IATHooker::HookAllModules("user32.dll", GetSysColorOrg, GetSysColorHooked);
	printf("Hooked %d calls to GetSysColor\n", count);

	wchar_t buf[128];
	swprintf(buf, 128, L"Hooked %d calls to GetSysColor", count);
	MessageBox(nullptr, buf, L"Debug", MB_OK);
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, PVOID lpReserved) {
	switch (reason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, 0, [](LPVOID)->DWORD {
			Sleep(100);
			HookFunctions();
			return 0;
			}, nullptr, 0, nullptr);

		wchar_t text[128];
		::StringCchPrintf(text, _countof(text), L"Injected into process %u", ::GetCurrentProcessId());
		::MessageBox(nullptr, text, L"DLL loaded! GetSysColor hooked!", MB_OK);

		break;

	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}