#include "ServiceInstaller.h"
#include "..\Utils\DebugOutput.h"

#define MIN_WAIT_TIME (1000)
#define MAX_WAIT_TIME (10000)

BOOL install(PWSTR pszServiceName,
	PWSTR pszDisplayName,
	DWORD dwStartType,
	PWSTR pszDependencies,
	PWSTR pszAccount,
	PWSTR pszPassword) {
	wchar_t szPath[MAX_PATH];
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	if (0 == GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath))) {
		debugOutputNum(L"Error in service install. GetModuleFileName failed (0x%08lx)\n", GetLastError());
		return FALSE;
	}

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS/*SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE*/);
	if (NULL == schSCManager) {
		debugOutputNum(L"Error in service install. OpenSCManager failed (0x%08lx)\n", GetLastError());
		return FALSE;
	}

	schService = CreateService(
		schSCManager,                   // SCManager database
		pszServiceName,                 // Name of service
		pszDisplayName,                 // Name to display
		SERVICE_ALL_ACCESS /*SERVICE_QUERY_STATUS*/, // Desired access
		SERVICE_WIN32_SHARE_PROCESS,    // Service type 
		dwStartType,                    // Service start type
		SERVICE_ERROR_NORMAL,           // Error control type
		szPath,                         // Service's binary
		NULL,                           // No load ordering group
		NULL,                           // No tag identifier
		pszDependencies,                // Dependencies
		pszAccount,                     // Service running account
		pszPassword                     // Password of the account
	);
	if (schService == NULL){
		debugOutputNum(L"Error in service install. CreateService failed (0x%08lx)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
		return FALSE;
	}

	CloseServiceHandle(schSCManager);
	CloseServiceHandle(schService);

	schSCManager = NULL;
	schService = NULL;
	debugOutputStr(L"Service %s is installed successfully.\n", pszServiceName);
	return TRUE;
}

DWORD calcWaitTime(DWORD dwWaitHint) {
	DWORD dwWaitTime = dwWaitHint / 10;

	if (dwWaitTime < MIN_WAIT_TIME)
		dwWaitTime = MIN_WAIT_TIME;
	else if (dwWaitTime > MAX_WAIT_TIME)
		dwWaitTime = MAX_WAIT_TIME;
	return dwWaitTime;
}

BOOL uninstall(PWSTR pszServiceName) {
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS ssSvcStatus = {0};
	BOOL retVal = FALSE;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (schSCManager == NULL){
		debugOutputNum(L"Error in server uninstall. OpenSCManager failed (0x%08lx)\n", GetLastError());
		return FALSE;
	}

	schService = OpenService(schSCManager, pszServiceName, SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
	if (schService == NULL) {
		debugOutputNum(L"Error in server uninstall. OpenService failed (0x%08lx)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		schSCManager = NULL;
		return FALSE;
	}

	if (ControlService(schService, SERVICE_CONTROL_STOP, &ssSvcStatus)) {
		debugOutputStr(L"Stopping %s...\n", pszServiceName);
		Sleep(10);

		while (QueryServiceStatus(schService, &ssSvcStatus)) {
			if (ssSvcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
				DWORD dwWaitTime = calcWaitTime(ssSvcStatus.dwWaitHint);
				Sleep(dwWaitTime);
			} else {
				break;
			}
		}

		if (ssSvcStatus.dwCurrentState == SERVICE_STOPPED) {
			debugOutputStr(L"%s has stopped.\n", pszServiceName);
		} else {
			debugOutputStr(L"Error in server uninstall. Failed to stop %s.\n", pszServiceName);
		}
	}

	if (!DeleteService(schService)) {
		debugOutputNum(L"Error in server uninstall. DeleteService failed (0x%08lx)", GetLastError());
		// not really required, but with some faith in the compiler, the following 
		// line will not be present in the binary
		retVal = FALSE; 
	} else {
		debugOutputStr(L"%s is removed.\n", pszServiceName);
		retVal = TRUE;
	}

	CloseServiceHandle(schSCManager);
	CloseServiceHandle(schService);
	schSCManager = NULL;
	schService = NULL;

	return retVal;
}

BOOL queryServiceStatus(SERVICE_STATUS_PROCESS* ssStatus, const SC_HANDLE schService) {
	DWORD dwBytesNeeded;
	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE)ssStatus,               // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded)) {              // size needed if buffer is too small

		printf("Error in queryServiceStatus. QueryServiceStatusEx failed (0x%08lx)\n", GetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL CtrlHandler(DWORD fdwCtrlType) {
	return TRUE;
}

BOOL runService(const LPCWSTR szSvcName) {
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;
	SERVICE_STATUS_PROCESS ssStatus = { 0 };
	BOOL retVal = FALSE;

	//if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE)) {
	//	printf("WTF?!");
	//}

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if (NULL == schSCManager){
		printf("Error in run. OpenSCManager failed (0x%08lx)\n", GetLastError());
		return FALSE;
	}

	schService = OpenService(schSCManager, szSvcName, SERVICE_ALL_ACCESS);

	if (schService == NULL) {
		printf("Error in run. OpenService failed (0x%08lx)\n", GetLastError());
		CloseServiceHandle(schSCManager);
		return FALSE;
	}

	if (!queryServiceStatus(&ssStatus, schService)) {
		goto cleanup;
	}

	// Check if the service is already running.
	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
		printf("Service is already running\n");
		goto cleanup;
	}

	// Wait for the service to stop.
	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
		DWORD dwWaitTime = calcWaitTime(ssStatus.dwWaitHint);
		Sleep(dwWaitTime);
		// Check the status until the service is no longer stop pending. 
		if (!queryServiceStatus(&ssStatus, schService)) {
			goto cleanup;
		}
	}

	DWORD ret = StartService(schService, 0, NULL);
	if (!ret) {
		printf("Error in run. StartService failed (0x%08lx)\n", GetLastError());
		goto cleanup;
	} else {
		printf("Service start pending...\n");
	}

	// Check the status until the service is no longer start pending. 
	if (!queryServiceStatus(&ssStatus, schService)) {
		goto cleanup;
	}

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
		Sleep(10);
		if (!queryServiceStatus(&ssStatus, schService)) {
			goto cleanup;
		}
	}
	if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
		retVal = TRUE;
		printf("Service started successfully.\n");
	} else {
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState);
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
	}
	
cleanup: // cleaning up after us
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
	return retVal;
}