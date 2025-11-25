// DLL Injection + IAT Hooking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>
#include <stdio.h>

int Error(const char* msg) {
	printf("%s (%u)\n", msg, ::GetLastError());

	return 1;
}

int main(int argc, const char* argv[]) {
	if (argc < 3) {
		printf("Usage: injector <pid> <dllpath>\n");

		return 0;
	}

	HANDLE hProcess = ::OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_CREATE_THREAD,
		FALSE, atoi(argv[1]));
	if (!hProcess)
		return Error("Failed to open process");

	// SetEnvironmentVariableW(L"HANDLE_OF_NOTEPAD", (LPCWSTR)argv[1]);

	void* buffer = ::VirtualAllocEx(hProcess, nullptr, 1 << 12,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!buffer)
		return Error("Failed to allocate buffer in target process");

	if (!::WriteProcessMemory(hProcess, buffer, argv[2], ::strlen(argv[2]) + 1, nullptr))
		return Error("Failed to write to target process");

	DWORD tid = 0;
	HANDLE hThread = nullptr;
	auto hKernel32 = ::GetModuleHandle(L"kernel32.dll");
	if (hKernel32) {
		
		hThread = ::CreateRemoteThread(hProcess, nullptr, 0,
			(LPTHREAD_START_ROUTINE)::GetProcAddress(hKernel32, "LoadLibraryA"),
			buffer, 0, &tid);
	}

	if (!hThread)
		return Error("Failed to create remote thread");
	

	/*DWORD exitCode = 0;
	WaitForSingleObject(hThread, 5000);
	GetExitCodeThread(hThread, &exitCode);
	printf("LoadLibrary in remote process, returned 0x%p\n", (void*)exitCode);*/

	printf("Thread %u created successfully!\n", tid);
	if (WAIT_OBJECT_0 == ::WaitForSingleObject(hThread, 5000))
		printf("Thread exited.\n");
	else
		printf("Thread still hanging around...\n");

	// be nice
	::VirtualFreeEx(hProcess, buffer, 0, MEM_RELEASE);

	::CloseHandle(hThread);
	::CloseHandle(hProcess);

	return 0;
}