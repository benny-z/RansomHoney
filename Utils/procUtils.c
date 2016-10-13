#include "procUtils.h"

BOOL getCurProcArchitecture() {
#if defined(_WIN64)
	return ARCH_64;  // 64-bit programs run only on Win64
#elif defined(_WIN32)
	BOOL f64 = FALSE;
	return (IsWow64Process(GetCurrentProcess(), &f64) && f64) ? ARCH_32 : ARCH_64;
#endif
}

BOOL getAllProcsIds(DWORD procIds[], DWORD sizeOfProcIds, DWORD* numOfProcs) {
	DWORD cbNeeded = -1;
	if (!EnumProcesses(procIds, sizeOfProcIds, &cbNeeded) || -1 == cbNeeded) {
		debugOutputNum(L"Error in getAllProcsIds: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	*numOfProcs = cbNeeded / sizeof(DWORD);
	return TRUE;
}

DWORD getPorcIdByName(const wchar_t* procName) {
	HANDLE hProc = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProc) {
		debugOutputNum(L"Error in getPorcIdByName. CreateToolhelp32Snapshot failed (0x%08lx)", GetLastError());
		return -1;
	}

	PROCESSENTRY32 pe32 = { 0 };
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProc, &pe32)) {
		CloseHandle(hProc);
		debugOutputNum(L"Error in getPorcIdByName. Process32First failed (0x%08lx)", GetLastError());
		return -1;
	}

	do {
		if (0 == wcscmp(procName, pe32.szExeFile)) {
			CloseHandle(hProc);
			return pe32.th32ProcessID;
		}
	} while (Process32Next(hProc, &pe32));

	debugOutputStr(L"No process found with the name %s", procName);
	CloseHandle(hProc);

	return -1;
}

architecture getProcArchitecture(DWORD procId, HANDLE hProc){
	HANDLE internalHProc = INVALID_HANDLE_VALUE;
	architecture retVal = -1;
	if (NULL == hProc) {
		internalHProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procId);
		if (NULL == internalHProc) {
			debugOutputNum(L"Error in getProcArchitecture. OpenProcess failed (%d)", GetLastError());
			return -1;
		}
	} else {
		internalHProc = hProc;
	}
	BOOL isWow64Process = -1;
	if (!IsWow64Process(internalHProc, &isWow64Process)) {
		debugOutputNum(L"Error in getProcArchitecture. IsWow64Process failed (%d)", GetLastError());
		retVal = -1;
		goto cleanup;
	}
	retVal = (isWow64Process) ? ARCH_32 : ARCH_64;
cleanup:
	if (NULL == hProc) {
		CloseHandle(internalHProc);
	}
	return retVal;
}


/*BOOL ISPROCSUSPENDED(DWORD PROCID) {
	HANDLE HPROC = OPENPROCESS(PROCESS_ALL_ACCESS, FALSE, PROCID);
	IF (NULL == HPROC) {
		DEBUGOUTPUTNUM(L"ERROR IN ISPROCSUSPENDED. FAILED TO RUN OPENPROCESS (0X%08LX)", GETLASTERROR());
		RETURN FALSE;
	}
	PPROCESS_BASIC_INFORMATION PBI = { 0 };
	DWORD PROCINFOSIZE = SIZEOF(PPROCESS_BASIC_INFORMATION);
	DWORD DWSIZENEEDED = INVALID_SIZE;
	NTQUERYINFORMATIONPROCESS(HPROC, PROCESSBASICINFORMATION, &PBI, PROCINFOSIZE, &DWSIZENEEDED));
	IF (INVALID_SIZE == DWSIZENEEDED || 0 == DWSIZENEEDED) {
		DEBUGOUTPUTNUM(L"ERROR IN ISPROCSUSPENDED. FAILED TO RUN NTQUERYINFORMATIONPROCESS (0X%08LX)", GETLASTERROR());
		CLOSEHANDLE(HPROC);
		RETURN FALSE;
	}
	


	CLOSEHANDLE(HPROC);
} */