#include <Windows.h>
#include <stdio.h>
#include "..\RansomHoney\injector.h"
#include "..\RansomHoney\hooker.h"
#include "..\RansomHoney\RansomHoney.h"
#include "..\RansomHoney\commonFilesList.h"

#define DUMMY_DLL L"C:\\my_projects\\RansomHoney\\"
#define DUMMY_DLL64 DUMMY_DLL L"x64\\Debug\\DummyDLL64.dll"
#define DUMMY_DLL32 DUMMY_DLL L"Debug\\DummyDLL32.dll"

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
#if defined(_WIN64)
	HMODULE hLibrary = LoadLibrary(FILE_WATCHER_64_DLL);
#elif defined(_WIN32)
	HMODULE hLibrary = LoadLibrary(FILE_WATCHER_32_DLL);
#endif 

	if (NULL == hLibrary) {
		wprintf(L"Failed to get the library %s", FILE_HIDER_64_DLL);
		return -1;
	}

	CreateFileW(L"C:\\temp_file_do_not_touch.docx", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	FreeLibrary(hLibrary);
	//CreateFileW(L"filename", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	return 0;
}

int fileHiderLocalTest() {
	WIN32_FIND_DATAA FindFileData;
	FindFirstFileA("test_string", &FindFileData);
	HMODULE hLibrary = LoadLibrary(FILE_HIDER_64_DLL);

	if (NULL == hLibrary) {
		wprintf(L"Failed to get the library %s", FILE_HIDER_64_DLL);
		return -1;
	}
	listFilesInDir(L"C:\\*");
	FreeLibrary(hLibrary);
	CreateFileA(L"filename", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	return 0;
}

int injectDummyToProc() {
	// make sure notepad.exe is running
	DWORD processId = getPorcIdByName(L"notepad.exe");
	TCHAR* dllToInject = (ARCH_64 == getProcArchitecture(processId, NULL)) ? DUMMY_DLL64 : DUMMY_DLL32;

	LPVOID pInjectedDllAddr = injectToProc(dllToInject, processId);
	if (NULL == pInjectedDllAddr) {
		wprintf(L"Failed to inject DLL into process 0x%08lx. Last error: 0x%08lx\n", processId, GetLastError());
		return -1;
	}
	if (!runInjectedDLL(processId, pInjectedDllAddr, dllToInject)) {
		wprintf(L"Failed to run thread for process 0x%08lx\n", processId);
		return -1;
	}
	printf("Great success! (0x%08lx)", processId);
	return 0;
}

int hookRemoteFileWatcherTest() {
	// make sure notepad.exe is running
	DWORD processId = getPorcIdByName(L"notepad.exe");
	const TCHAR* dllToInject = (ARCH_64 == getProcArchitecture(processId, NULL)) ? FILE_WATCHER_64_DLL : FILE_WATCHER_32_DLL;
	LPVOID pInjectedDllAddr = injectToProc(dllToInject, processId);
	if (NULL == pInjectedDllAddr) {
		wprintf(L"Failed to inject DLL into process 0x%08lx. Last error: 0x%08lx\n", processId, GetLastError());
		return -1;
	}
	if (!runInjectedDLL(processId, pInjectedDllAddr, FILE_WATCHER_64_DLL)) {
		wprintf(L"Failed to run thread for process 0x%08lx\n", processId);
		return -1;
	}
	printf("Great success! (0x%08lx)", processId);
	return 0;
}

int injectToAllProcsDummyTest() {
	return injectToAllProcs(DUMMY_DLL64, DUMMY_DLL32);
}

int injectToAllProcsHookerTest() {
	return injectToAllProcs(FILE_WATCHER_64_DLL, FILE_WATCHER_32_DLL);
}

int simpleInjectionTest() {
	DWORD processId = GetCurrentProcessId();
	const TCHAR* dllToInject = (ARCH_64 == getProcArchitecture(processId, NULL)) ? DUMMY_DLL64 : DUMMY_DLL32;
	LPVOID pInjectedDllAddr = injectToProc(dllToInject, processId);
	if (NULL == pInjectedDllAddr) {
		printf("Failed to inject DLL into process 0x%08lx. Last error: 0x%08lx\n", processId, GetLastError());
		return -1;
	}
	if (!runInjectedDLL(processId, pInjectedDllAddr, dllToInject)) {
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
	return injectAndRun(FILE_HIDER_64_DLL, cmdProcId);
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
	return injectAndRun(FILE_HIDER_64_DLL, cmdProcId);
}

int injectDummyIntoExplorerExe() {
	DWORD explorerProcId = getPorcIdByName(L"explorer.exe");
	TCHAR* dllToInject = (ARCH_64 == getProcArchitecture(explorerProcId, NULL)) ? DUMMY_DLL64 : DUMMY_DLL32;
	if (-1 == explorerProcId) {
		return FALSE;
	}
	return injectAndRun(dllToInject, explorerProcId);
}

void dummy() {}

int hookNonExistingFunctionTest() {
	HookData nonExistingFuncData = { dummy, CreateFileA, NULL, "NonExistingFunction1337", NULL, FALSE };
	HMODULE hModule = GetModuleHandleA(NULL);
	return setHook(&nonExistingFuncData, hModule);

};

int main() {
	//return hookCmdExeTest();
	//return hookExplorerExeTest();
	//return hookNotepadFilerHiderTets();
	//return injectDummyIntoExplorerExe();
	//return fileHiderLocalTest();
	//CreateFileW(L"C:\\temp_file_do_not_touch.docx", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	return fileWatcherLocalTest();
	//return hookRemoteFileWatcherTest();
	//return hookNonExistingFunctionTest();
	//return simpleInjectionTest();
	//return injectDummyToProc();
	//return injectToAllProcsDummyTest();
	//return injectToAllProcsHookerTest();
	//return 0;
 }