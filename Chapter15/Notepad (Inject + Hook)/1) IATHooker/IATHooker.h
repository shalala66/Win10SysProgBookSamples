#pragma once

struct IATHooker final abstract {
	static int HookFunction(PCWSTR callerModule, PCSTR moduleName, PVOID originalProc, PVOID hookProc);
	static int HookAllModules(PCSTR moduleName, PVOID originalProc, PVOID hookProc);
};
