#include <Windows.h>
#include <stdio.h>
#include "..\RansomHoney\injector.h"
#include "..\RansomHoney\hooker.h"
#include "..\RansomHoney\RansomHoney.h"
#include "..\RansomHoney\commonFilesList.h"

#define DUMMY_DLL (L"C:\\my_projects\\projects\\x64\\Debug\\DummyDLL.dll")
#define FILE_WATCHER_DLL (L"C:\\my_projects\\projects\\x64\\Debug\\fileWatcher.dll")
#define FILE_HIDER_DLL (L"C:\\my_projects\\projects\\x64\\Debug\\fileHider.dll")

void listFilesInDir(TCHAR szDir[MAX_PATH]) {
	// source code kindly borrowed from here:
	// https://msdn.microsoft.com/en-us/library/windows/desktop/aa365200(v=vs.85).aspx
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;

	hFind = FindFirstFile(szDir, &ffd);

	if (INVALID_HANDLE_VALUE == hFind) {
		printf("error in FindFirstFile (%d)", GetLastError());
		return;
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			wprintf(L"  %s   <DIR>\n", ffd.cFileName);
		}
		else {
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
		}
	} while (FindNextFile(hFind, &ffd) != 0);

	FindClose(hFind);
}

int fileWatcherLocalTest() {
	HMODULE hLibrary = LoadLibrary(FILE_WATCHER_DLL);

	if (NULL == hLibrary) {
		wprintf(L"Failed to get the library %s", FILE_HIDER_DLL);
		return -1;
	}

	CreateFileW(L"C:\\temp_file_do_not_touch.docx", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	FreeLibrary(hLibrary);
	CreateFileW(L"filename", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	return 0;
}

int fileHiderLocalTest() {
	WIN32_FIND_DATAA FindFileData;
	FindFirstFileA("test_string", &FindFileData);
	HMODULE hLibrary = LoadLibrary(FILE_HIDER_DLL);

	if (NULL == hLibrary) {
		wprintf(L"Failed to get the library %s", FILE_HIDER_DLL);
		return -1;
	}
	listFilesInDir(L"C:\\*");
	FreeLibrary(hLibrary);
	CreateFileW(L"filename", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	return 0;
}

int injectDummyToProc() {
	// make sure notepad.exe is running
	DWORD processId = getPorcIdByName(L"notepad.exe");
	LPVOID pInjectedDllAddr = injectToProc(DUMMY_DLL, processId);
	if (NULL == pInjectedDllAddr) {
		wprintf(L"Failed to inject DLL into process 0x%08lx. Last error: 0x%08lx\n", processId, GetLastError());
		return -1;
	}
	if (!runInjectedDLL(processId, pInjectedDllAddr, DUMMY_DLL)) {
		wprintf(L"Failed to run thread for process 0x%08lx\n", processId);
		return -1;
	}
	printf("Great success! (0x%08lx)", processId);
	return 0;
}

int hookRemoteFileWatcherTest() {
	// make sure notepad.exe is running
	DWORD processId = getPorcIdByName(L"notepad.exe");
	LPVOID pInjectedDllAddr = injectToProc(FILE_WATCHER_DLL, processId);
	if (NULL == pInjectedDllAddr) {
		wprintf(L"Failed to inject DLL into process 0x%08lx. Last error: 0x%08lx\n", processId, GetLastError());
		return -1;
	}
	if (!runInjectedDLL(processId, pInjectedDllAddr, FILE_WATCHER_DLL)) {
		wprintf(L"Failed to run thread for process 0x%08lx\n", processId);
		return -1;
	}
	printf("Great success! (0x%08lx)", processId);
	return 0;
}

int injectToAllProcsDummyTest() {
	return injectToAllProcs(DUMMY_DLL);
}

int injectToAllProcsHookerTest() {
	return injectToAllProcs(FILE_WATCHER_DLL);
}

int simpleInjectionTest() {
	DWORD processId = GetCurrentProcessId();
	LPVOID pInjectedDllAddr = injectToProc(DUMMY_DLL, processId);
	if (NULL == pInjectedDllAddr) {
		printf("Failed to inject DLL into process 0x%08lx. Last error: 0x%08lx\n", processId, GetLastError());
		return -1;
	}
	if (!runInjectedDLL(processId, pInjectedDllAddr, DUMMY_DLL)) {
		printf("Failed to run thread for process 0x%08lx\n", processId);
		return -1;
	}
	printf("Great success! (0x%08lx)", processId);
	return 0;
}

int hookCmdExeTest() {
	createFiles();
	DWORD cmdProcId = getPorcIdByName(L"cmd.exe");
	if (-1 == cmdProcId) {
		return FALSE;
	}
	return injectAndRun(FILE_HIDER_DLL, cmdProcId);
}

int hookExplorerExeTest() {
	createFiles();
	hideFiles();
	return 0;
}

int hookNotepadFilerHiderTets() {
	createFiles();
	DWORD cmdProcId = getPorcIdByName(L"notepad.exe");
	if (-1 == cmdProcId) {
		return FALSE;
	}
	return injectAndRun(FILE_HIDER_DLL, cmdProcId);
}

int injectDummyIntoExplorerExe() {
	DWORD explorerProcId = getPorcIdByName(L"explorer.exe");
	if (-1 == explorerProcId) {
		return FALSE;
	}
	return injectAndRun(DUMMY_DLL, explorerProcId);
}

int main() {
	//return hookCmdExeTest();
	//return hookExplorerExeTest();
	//return hookNotepadFilerHiderTets();
	//return injectDummyIntoExplorerExe();
	return fileHiderLocalTest();
	//return fileWatcherLocalTest();
	//return hookRemoteFileWatcherTest();
	//return simpleInjectionTest();
	//return injectDummyToProc();
	//return injectToAllProcsDummyTest();
	//return injectToAllProcsHookerTest();
	//return 0;
 }