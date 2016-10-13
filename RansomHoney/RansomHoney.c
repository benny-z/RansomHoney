#include "RansomHoney.h"

BOOL createFiles() {
	for (int i = 0; i < NUM_OF_FILES; ++i) {
		const TCHAR* filename = getFilesList()[i];
		TCHAR expandedFname[MAX_PATH] = { 0 };
		if (0 == ExpandEnvironmentStrings(filename, expandedFname, MAX_PATH)) {
			wprintf(L"Error in createFiles. ExpandEnvironmentStrings failed (0x%08lx)", GetLastError());
			return FALSE;
		}
		HANDLE hFile = CreateFile(expandedFname, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_HIDDEN, NULL);
		if (INVALID_HANDLE_VALUE == hFile) {
			DWORD lastError = GetLastError();
			if (ERROR_FILE_EXISTS == lastError) {
				continue;
			}
			wprintf(L"Error in createFiles. CreateFile failed (0x%08lx)", lastError);
			return FALSE;
		}
		CloseHandle(hFile);
	}
	return TRUE;
}

BOOL initFileWatcher() {
	return injectToAllProcs(FILE_WATCHER_64_DLL, FILE_WATCHER_32_DLL);
}

BOOL hideFiles() {
	for (int i = 0; i < NUM_OF_HIDE_FROM_PROCS; ++i) {
		const TCHAR* procName = getProcsToHideFrom()[i];
		DWORD procId = getPorcIdByName(procName);
		if (-1 == procId) {
			debugOutputStr(L"Error in hideFiles. Process %s is not running", procName);
			continue;
		}
		DWORD isX64 = getProcArchitecture(procId, NULL);
		if (-1 == isX64) {
			debugOutputNum(L"Error in hideFiles. isProc64 failed (%d)", GetLastError());
			continue;
		}
		TCHAR* dllToRun = (ARCH_64 == isX64) ? FILE_HIDER_64_DLL : FILE_HIDER_32_DLL;
		if (!injectAndRun(dllToRun, procId)) {
			return FALSE; // the debug message was printed internally
		}
	}
	return TRUE;
}

BOOL isFileHiddenA(const char* filename) {
	wchar_t wFilename[MAX_PATH] = { 0 };
	if (-1 == swprintf(wFilename, MAX_PATH, L"%hs", filename)) {
		debugOutputStr(L"Error in isFileHiddenA. swprintf failed for the string %s", filename);
		return FALSE;
	}
	return isFileHiddenW(wFilename);
}

BOOL isFileHiddenW(const wchar_t* curFullpath) {
	const TCHAR** filesLst = getFilesList();
	for (int i = 0; i < NUM_OF_FILES; ++i) {
		const TCHAR* fileFullpath = filesLst[i];
		TCHAR* hiddenFilename = PathFindFileName(fileFullpath);
		TCHAR* filename = PathFindFileName(curFullpath);
		if (0 == _wcsicmp(hiddenFilename, filename)) {
			debugOutputStr(L"File %s is hidden", filename);
			return TRUE;
		}
	}
	return FALSE;
}
