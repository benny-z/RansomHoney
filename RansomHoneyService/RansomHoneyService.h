#ifndef __RANSOM_HONEY_SERVICE_H__
#define __RANSOM_HONEY_SERVICE_H__

#include <Windows.h>
#include "..\Utils\DebugOutput.h"
#include "..\Utils\procUtils.h"

#define RH_SERVICE_NAME          L"RansomHoneyService"
#define RH_SERVICE_DISPLAY_NAME  L"RansomHoneyService"
#define RH_SERVICE_START_TYPE    SERVICE_DEMAND_START
#define RH_SERVICE_DEPENDENCIES  L""
#define RH_SERVICE_ACCOUNT       L".\\LocalSystem"
#define RH_SERVICE_PASSWORD      NULL

/**
 * @brief      Entry function of this service
 *
 * @param[in]  dwArgc   Number of arguments this method can get.
 * @param      pszArgv  List of all the string arguments this service got.
 * 
 * @note       The arguments are not yet used here.
 */
VOID WINAPI ServiceMain(DWORD dwArgc, PWSTR *pszArgv);

/**
 * @brief      A console process uses this function to handle control signals received 
 *             by the process. When the signal is received, the system creates a new 
 *             thread in the process to execute the function (from MSDN:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/ms683242(v=vs.85).aspx)
 *
 * @param[in]  dwCtrl  The type of control signal received by the handler. 
 */
VOID WINAPI serviceCtrlHandler(DWORD dwCtrl);

/**
 * @brief      The functions that should run when the service starts.
 */
VOID onStart();

/**
 * @brief      The functions that should run when the service stops.
 */
VOID onStop();

/**
 * @brief      The functions that should run when the system shuts down.
 * 
 * @note       The function is not yet implemented as there are no measurements 
 *             to be taken once the system shuts down.
 */
VOID onShutdown();

/**
 * @brief      The functions that should run when the server is paused.
 * 
 * @note       The function is not yet implemented.
 */
VOID onPause();

/**
 * @brief      The functions that should run when the server is resumed.
 * 
 * @note       The function is not yet implemented.
 */
VOID onContinue();

/**
 * @brief      A warper of the WinApi function SetServiceStatus
 *
 * @param[in]  statusHandle     A handle to the status information 
 *                              structure for the current service. 
 * @param[in]  dwCurrentState   The current state of the service.
 * @param[in]  dwWin32ExitCode  The error code the service uses to report an
 *                              error that occurs when it is starting or stopping. 
 * @param[in]  dwWaitHint       The estimated time required for a pending start, stop, 
 *                              pause, or continue operation, in milliseconds. 
 * @note       More detailed documentation regarding the last three parameters can be found in the 
 *             SERVICE_STATUS structure's documentation here:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/ms685996(v=vs.85).aspx
 * @return     TRUE iff the procedure ended successfully.
 */
BOOL setServiceStatus(SERVICE_STATUS_HANDLE statusHandle,
	DWORD dwCurrentState,
	DWORD dwWin32ExitCode,
	DWORD dwWaitHint);

/**
 * @brief      Makes sure that fileWatcher is injected into new processes as well.
 * 
 * @note       YET TO BE IMPLEMENTED
 * @return     
 */
DWORD startWatchDog();

#endif // __RANSOM_HONEY_SERVICE_H__