#include "RansomHoneyService.h"
#include "..\RansomHoney\RansomHoney.h"

#define DW_WAIT_HINT_DEFAULT (3000)

SERVICE_STATUS_HANDLE g_statusHandle = NULL;

HANDLE g_serviceStopEvent = INVALID_HANDLE_VALUE;

SERVICE_STATUS g_serviceStatusStruct = { 0 };

BOOL run() {
	SERVICE_TABLE_ENTRY serviceTable[] = 
	{
		{ RH_SERVICE_NAME, ServiceMain },
		{ NULL, NULL }
	};
	return StartServiceCtrlDispatcher(serviceTable);
}

VOID WINAPI ServiceMain(DWORD  dwArgc, LPWSTR *pszArgv) {
	g_statusHandle = RegisterServiceCtrlHandler(RH_SERVICE_NAME, serviceCtrlHandler);
	if (NULL == g_statusHandle) {
		debugOutputNum(L"Error in ServiceMain. RegisterServiceCtrlHandler Failed (%d)", GetLastError());
		return;
	}
	setServiceStatus(g_statusHandle, SERVICE_START_PENDING, NO_ERROR, DW_WAIT_HINT_DEFAULT);

	g_serviceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (NULL == g_serviceStopEvent) {
		debugOutputNum(L"Error in ServiceMain. CreateEvent failed (%d)", GetLastError());
		setServiceStatus(g_statusHandle, SERVICE_STOPPED, GetLastError(), 0);
		return;
	}
	setServiceStatus(g_statusHandle, SERVICE_RUNNING, NO_ERROR, 0);

	onStart();

	WaitForSingleObject(g_serviceStopEvent, INFINITE);
	setServiceStatus(g_statusHandle, SERVICE_STOPPED, NO_ERROR, 0);
	
	CloseHandle(g_serviceStopEvent);
}

BOOL setServiceStatus(SERVICE_STATUS_HANDLE statusHandle,
	DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint) {
	static DWORD dwCheckPoint = 1;

	g_serviceStatusStruct.dwServiceType = SERVICE_WIN32_SHARE_PROCESS; 
	g_serviceStatusStruct.dwCurrentState = dwCurrentState;
	g_serviceStatusStruct.dwWin32ExitCode = dwWin32ExitCode;
	g_serviceStatusStruct.dwWaitHint = dwWaitHint;

	if (SERVICE_START_PENDING == dwCurrentState) {
		g_serviceStatusStruct.dwControlsAccepted = 0;
	} else {
		g_serviceStatusStruct.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}

	g_serviceStatusStruct.dwCheckPoint =
		((SERVICE_RUNNING == dwCurrentState) ||
		(SERVICE_STOPPED == dwCurrentState)) ?
		0 : dwCheckPoint++;

	return SetServiceStatus(statusHandle, &g_serviceStatusStruct);
}

VOID WINAPI serviceCtrlHandler(DWORD dwCtrl) {
	switch (dwCtrl) {
	case SERVICE_CONTROL_STOP: onStop(); break;
	case SERVICE_CONTROL_PAUSE: onPause(); break;
	case SERVICE_CONTROL_CONTINUE: onContinue(); break;
	case SERVICE_CONTROL_SHUTDOWN: onShutdown(); break;
	default: break;
	}
}

VOID onStart() {
	debugOutput(L"In onStart");
	if (!initFilesList()) {
		debugOutput(L"initFilesList failed!");
		return;
	}
	if (!createFiles()) {
		debugOutput(L"createFiles failed!");
		return;
	}
	if (!hideFiles()) {
		debugOutput(L"hideFiles failed!");
		return;
	}
	if (!initFileWatcher()) {
		debugOutput(L"initFileWatcher failed!");
		return;
	}
	if (!startWatchDog(g_serviceStopEvent)) {
		debugOutput(L"startWatchDog failed!");
		return;
	}
	onStop();
}

VOID onStop() {
	if (SERVICE_RUNNING != g_serviceStatusStruct.dwCurrentState) {
		return;
	}

	SetEvent(g_serviceStopEvent);

	setServiceStatus(g_statusHandle, SERVICE_STOP_PENDING, NO_ERROR, 0);
}

VOID onShutdown() {
	/* Left blank on purpose */
}

VOID onPause() {
	/* Left blank on purpose */
}

VOID onContinue() {
	/* Left blank on purpose */
}

VOID onInterrogate() {
	/* Left blank on purpose */
}
