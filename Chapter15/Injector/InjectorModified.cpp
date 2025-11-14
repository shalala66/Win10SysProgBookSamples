#include <Windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>

int Error(const wchar_t* msg) {
    wprintf(L"%s (GetLastError=%u)\n", msg, ::GetLastError());
    return 1;
}


bool SearchModule(DWORD pid, const wchar_t* dllName) {
    bool found = false;
    HANDLE snap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

    if (snap == INVALID_HANDLE_VALUE)
        return false;

    MODULEENTRY32W me;
    me.dwSize = sizeof(me);

    if (::Module32FirstW(snap, &me)) {
        do {
			wprintf(L"Module name: \"%s\" \n", me.szModule);

            if (_wcsicmp(me.szModule, dllName) == 0) {
                found = true;
                // break;
            }
        } while (::Module32NextW(snap, &me));
    }

    ::CloseHandle(snap);
    return found;
}


int wmain(int argc, wchar_t* argv[]) {
	if (argc < 3) {
		printf("Usage: injector <pid> <dllpath>\n");
		return 0;
	}

	DWORD pid = (DWORD)_wtoi(argv[1]);
	const wchar_t* dllPath = argv[2];


	HANDLE hProcess = ::OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD,
		FALSE, pid);
	if (!hProcess)
		return Error(L"Failed to open process");


	void* buffer = ::VirtualAllocEx(hProcess, nullptr, 1 << 12,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!buffer)
		return Error(L"Failed to allocate buffer in target process");


	if (!::WriteProcessMemory(hProcess, buffer, dllPath, (wcslen(dllPath) + 1) * sizeof(wchar_t), nullptr))
		return Error(L"Failed to write to target process");


	DWORD tid;
	HANDLE hThread = ::CreateRemoteThread(hProcess, nullptr, 0,
		(LPTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(L"kernel32.dll"), "LoadLibraryW"),
		buffer, 0, &tid);
	if (!hThread)
		return Error(L"Failed to create remote thread");


	wprintf(L"Remote thread %u created succesfully, waiting for it to finish...\n", tid);
	WaitForSingleObject(hThread, 2000);


	const wchar_t* dllNamePtr = wcsrchr(dllPath, L'\\');
	if (!dllNamePtr) {
		dllNamePtr = dllPath;
	}
	else {
		dllNamePtr++;
	}


	if (SearchModule(pid, dllNamePtr))
		wprintf(L"Success: DLL \"%s\" loaded!\n", dllNamePtr);
	else
		wprintf(L"Failure: DLL \"%s\" doesn't loaded!\n", dllNamePtr);


	if (WAIT_OBJECT_0 == ::WaitForSingleObject(hThread, 1000))
		printf("Remote thread exited.\n");
	else
		printf("Remote thread still hanging around...\n");


	::VirtualFreeEx(hProcess, buffer, 0, MEM_RELEASE);

	::CloseHandle(hThread);
	::CloseHandle(hProcess);

	return 0;
}
