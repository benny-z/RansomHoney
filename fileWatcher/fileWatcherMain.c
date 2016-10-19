#include "fileWatcherMain.h"

#define NUM_OF_FUNCS_TO_HOOK_WATCHER (3)
#define CREATE_FILE_A        (0)
#define CREATE_FILE_W        (1)
#define GET_FILE_TYPE        (2)

typedef HANDLE(WINAPI *createFileAPtr)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI *createFileWPtr)(LPCWSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef int (WINAPI *messageBoxAPtr)(HWND, LPCSTR, LPCSTR, UINT);
typedef DWORD (WINAPI *getFileTypePtr)(HANDLE);

HookData g_funcsToHookWatcher[NUM_OF_FUNCS_TO_HOOK_WATCHER];
static BOOL isWatcherFuncsToHookInit = FALSE;

HMODULE g_user32lib = INVALID_HANDLE_VALUE;
messageBoxAPtr g_msgBoxFunc = NULL;

static BOOL isUsrIOInit = FALSE;
BOOL initUserIO() {
	if (!isUsrIOInit) {
		isUsrIOInit = TRUE;
		g_user32lib = LoadLibrary(L"user32.dll");
		if (!g_user32lib) {
			debugOutputNum(L"Error in initUserIO. LoadLibary failed (%d)", GetLastError());
			return FALSE;
		}
		g_msgBoxFunc = (messageBoxAPtr)GetProcAddress(g_user32lib, "MessageBoxA");
		return TRUE;
	}
	return TRUE;
}

BOOL queryUser(const CHAR* errorMsg) {
	int msgId = g_msgBoxFunc(NULL, errorMsg, "Warning", MB_YESNO);
	switch (msgId) {
	case IDYES:
		return FALSE;
	case IDNO:
		return TRUE;
	}
	return FALSE; // Just in case
}

DWORD
WINAPI
myGetFileType(HANDLE hFile) {
	TCHAR filepath[MAX_PATH] = { 0 };
	BOOL proceedToApiCall = TRUE;
	HANDLE hCurrentThread = INVALID_HANDLE_VALUE;

	if (getFilenameByHandle(hFile, (LPTSTR)&filepath)) {
		if (isFileHiddenW(filepath)) {
			proceedToApiCall = queryUser("GetFileType called on an unauthorized file. \nWould you like to suspend the caller thread?");
		}
	}
	if (proceedToApiCall) {
		return ((getFileTypePtr)g_funcsToHookWatcher[GET_FILE_TYPE].origFuncPtr)(hFile);
	} 
	hCurrentThread = GetCurrentThread();
	if (INVALID_HANDLE_VALUE != hCurrentThread) {
		SuspendThread(hCurrentThread);
	}
	return FILE_TYPE_UNKNOWN;
}

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
		BOOL proceedToApiCall = queryUser("CreateFileA called on an unauthorized file. \nWould you like to suspend the caller thread?");
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
		BOOL proceedToApiCall = queryUser("CreateFileW called on an unauthorized file. \nWould you like to suspend the caller thread?");
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
		HookData _getFileType = { myGetFileType, GetFileType, NULL, "GetFileType", baseIATAddr, FALSE };

		g_funcsToHookWatcher[CREATE_FILE_A] = _createFileA;
		g_funcsToHookWatcher[CREATE_FILE_W] = _createFileW;
		g_funcsToHookWatcher[GET_FILE_TYPE] = _getFileType;

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
	HMODULE hModule = INVALID_HANDLE_VALUE;
	switch (reason) {
		case DLL_PROCESS_ATTACH:
			hModule = GetModuleHandleA(NULL);
			if (!initUserIO() || !initProcData() || !initFuncsToHook(hModule) || !initFilesList()) {
				return FALSE;
			}
			hookMultipleFuncs(g_funcsToHookWatcher, hModule, NUM_OF_FUNCS_TO_HOOK_WATCHER);
			break;
	}
	return TRUE;
}