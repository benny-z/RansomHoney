#include <Windows.h>
#include <stdio.h>
#include "..\Utils\DebugOutput.h"
#include "..\Utils\procUtils.h"

DWORD procId;
TCHAR szFileName[MAX_PATH];
static BOOL isDataInit = FALSE;
void initData() {
	if (!isDataInit) {
		procId = GetCurrentProcessId();
		GetModuleFileNameW(NULL, szFileName, MAX_PATH);
		isDataInit = TRUE;
	}
}
BOOL WINAPI DllMain(
	__in HINSTANCE  hInstance,
	__in DWORD      reason,
	__in LPVOID     reserved) {

	initData();

	switch (reason) {
	case DLL_PROCESS_ATTACH:
		debugOutputNum(L"In process no. 0x%08lx\n", procId);
		debugOutputStr(L"Process name: %s\n", szFileName);
		debugOutput(L"Dummy DLL attach called");
		break;
	case DLL_PROCESS_DETACH:
		debugOutput(L"Dummy DLL detach called");
		break;
	}
	return TRUE;
}	