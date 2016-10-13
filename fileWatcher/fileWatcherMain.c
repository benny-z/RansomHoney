#include "fileWatcherMain.h"

#define NUM_OF_FUNCS_TO_HOOK_WATCHER (2)
#define CREATE_FILE_A        (0)
#define CREATE_FILE_W        (1)

typedef HANDLE(WINAPI *createFileAPtr)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI *createFileWPtr)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

HookData g_funcsToHookWatcher[NUM_OF_FUNCS_TO_HOOK_WATCHER];
static BOOL isWatcherFuncsToHookInit = FALSE;

HANDLE
WINAPI
myCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile) {

	HANDLE hCurrentThread = INVALID_HANDLE_VALUE;
	BOOL proceedToApiCall = TRUE;
	if (isFileHiddenA(lpFileName)) {
		int msgId = MessageBoxA(NULL, "CreateFileA called on an unauthorized file. \nWould you like to suspend the caller thread?", "Warning", MB_YESNO);
		switch (msgId) {
		case IDYES:
			proceedToApiCall = FALSE;
			break;
		case IDNO:
			proceedToApiCall = TRUE;
			break;
		}
	}
	if (proceedToApiCall) {
		return ((createFileAPtr)g_funcsToHookWatcher[CREATE_FILE_A].origFuncPtr)(lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);
	} else {
		hCurrentThread = GetCurrentThread();
		if (INVALID_HANDLE_VALUE != hCurrentThread) {
			SuspendThread(hCurrentThread);
		}
		return INVALID_HANDLE_VALUE;
	}
}

HANDLE
WINAPI
myCreateFileW(
	LPCWSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile) {

	HANDLE hCurrentThread = INVALID_HANDLE_VALUE;
	BOOL proceedToApiCall = TRUE;
	if (isFileHiddenW(lpFileName)) {
		int msgId = MessageBoxA(NULL, "CreateFileW called on an unauthorized file. \nWould you like to suspend the caller thread?", "Warning", MB_YESNO);
		switch (msgId) {
		case IDYES:
			proceedToApiCall = FALSE;
			break;
		case IDNO:
			proceedToApiCall = TRUE;
			break;
		}
	}
	if (proceedToApiCall) {
		return ((createFileWPtr)g_funcsToHookWatcher[CREATE_FILE_W].origFuncPtr)(lpFileName,
			dwDesiredAccess,
			dwShareMode,
			lpSecurityAttributes,
			dwCreationDisposition,
			dwFlagsAndAttributes,
			hTemplateFile);
	} else {
		hCurrentThread = GetCurrentThread();
		if (INVALID_HANDLE_VALUE != hCurrentThread) {
			SuspendThread(hCurrentThread);
		}
		return INVALID_HANDLE_VALUE;
	}
}

BOOL initFuncsToHook(HMODULE hModule) {
	// Making sure the global g_funcsToHookWatcher wouldn't be initialized more than one time
	// as it contains the isHookSet indicator
	if (!isWatcherFuncsToHookInit) {
		LPVOID baseIATAddr = getIATAddr(hModule);
		if (NULL == baseIATAddr) {
			debugOutputNum(L"Hooker: Error in DllMain, failed to get image import descriptor (0x%08lx)\n", GetLastError());
			return FALSE;
		}

		HookData _createFileA = { myCreateFileA, CreateFileA, NULL, "CreateFileA", baseIATAddr, FALSE };
		HookData _createFileW = { myCreateFileW, CreateFileW, NULL, "CreateFileW", baseIATAddr, FALSE };

		g_funcsToHookWatcher[CREATE_FILE_A] = _createFileA;
		g_funcsToHookWatcher[CREATE_FILE_W] = _createFileW;

		isWatcherFuncsToHookInit = TRUE;
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
			hookMultipleFuncs(g_funcsToHookWatcher, hModule, NUM_OF_FUNCS_TO_HOOK_WATCHER);
			break;
	}
	return TRUE;
}