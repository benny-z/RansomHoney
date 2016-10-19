#include "RansomHoney.h"

#include "..\Watchdog\watchdog.h"

#ifdef __cplusplus
extern "C" BOOL injectToAndRunNewProc(DWORD processId);
#endif // __cplusplus

BOOL injectToAndRunFileWatcher(DWORD processId) {
	if (0 == processId){
		return FALSE;
	}
	architecture procArch = getProcArchitecture(processId, NULL);
	if (-1 == procArch) {
		debugOutputNum(L"Error in injectToAllProcs. getProcArchitecture failed (%d)", GetLastError());
		return FALSE;
	}
	else {
		TCHAR* dllToRun = (ARCH_64 == procArch) ? FILE_WATCHER_64_DLL : FILE_WATCHER_32_DLL;
		return injectAndRun(dllToRun, processId);
	}
}

BOOL startWatchDog(HANDLE serviceStopEvent) {
	return startWtchdg(injectToAndRunFileWatcher, serviceStopEvent);
}

BOOL createFiles() {
	DWORD numOfFiles = getNumOfFiles();
	for (unsigned int i = 0; i < numOfFiles; ++i) {
		const TCHAR* filename = getFilesList()[i];
		if (NULL == filename) {
			continue;
		}
		HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_HIDDEN, NULL);
		if (INVALID_HANDLE_VALUE == hFile) {
			DWORD lastError = GetLastError();
			if (ERROR_FILE_EXISTS == lastError) {
				CloseHandle(hFile);
				continue;
			}
			wprintf(L"Error in createFiles. CreateFile failed (0x%08lx)", lastError);
		}
		if (hFile) {
			CloseHandle(hFile);
		}
	}
	return TRUE;
}

BOOL initFiles() {
	return initFilesList();
}

BOOL initFileWatcher() {
	return injectToAllProcs(FILE_WATCHER_32_DLL, FILE_WATCHER_64_DLL);
}

BOOL hideFiles() {
	for (int i = 0; i < NUM_OF_HIDE_FROM_PROCS; ++i) {
		const TCHAR* procName = getProcsToHideFrom()[i];
		DWORD procId = getPorcIdByName(procName);
		if (-1 == procId) {
			debugOutputStr(L"Error in hideFiles. Process %s is not running", procName);
			continue;
		}
		architecture procArch = getProcArchitecture(procId, NULL);
		if (-1 == procArch) {
			debugOutputNum(L"Error in hideFiles. getProcArchitecture failed (%d)", GetLastError());
			continue;
		}
		if (procArch != getCurProcArchitecture()) {
			debugOutputStr(L"Error in hideFiles. Process %s and current process architectures do not match\n", procName);
			continue;
		}
		TCHAR* dllToRun = (ARCH_64 == procArch) ? FILE_HIDER_64_DLL : FILE_HIDER_32_DLL;
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
	for (unsigned int i = 0; i < getNumOfFiles(); ++i) {
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
