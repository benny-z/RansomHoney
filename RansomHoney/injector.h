#ifndef __INJECTOR_H__
#define __INJECTOR_H__

#include <Windows.h>
#include <stdio.h>

#include "..\Utils\DebugOutput.h"
#include "..\Utils\procUtils.h"

#define INJECTION_TIMEOUT (1000) // in milliseconds

/**
 * @brief      Injects and runs a dll specified in the arguments to all of the processes.
 * 
 * @note       Please note that it will inject only to processes that were compiled for the 
 *             same platform, i.e., if the injector is compiled for the x64 platform, it will 
 *             inject only to x64 processes.
 * 
 * @param[in]  dllPath  Path of the DLL to inject
 *
 * @return     TRUE iff the injection went OK. 
 *             Please note that a positive result from this function doesn't necessarily mean
 *             that the DLL was injected to ALL of the processes.
 */
BOOL injectToAllProcs(const TCHAR* dllPath);

/**
 * @brief      Injects and runs a dll specified in the arguments to the process with the Id 
 *             specified in the arguments.
 *
 * @param[in]  dllPath    Path of the DLL to inject
 * @param[in]  processId  The identifier of the process we would like to inject the DLL to.
 *
 * @return     TRUE iff the injection was successful
 */
BOOL injectAndRun(const TCHAR* dllPath, DWORD processId);

/**
 * @brief      Injects a dll specified in the arguments to the process with the Id 
 *             specified in the arguments.
 *             
 * @param[in]  dllPath    Path of the DLL to inject.
 * @param[in]  processId  The identifier of the process we would like to inject the DLL to.
 *
 * @return     TRUE iff the injection was successful
 */
LPVOID injectToProc(const TCHAR* dllPath, DWORD processId);

/**
 * @brief      Runs an already injected dll.
 * 
 * @note       This injection method relies of the fact that kernel32.dll ALWAYS loads to the 
 *             same address. Thus, this injection method might not be feasible because of ASLR.
 *             All in all, it's the end of 2016 and it's still working.
 *
 * @param[in]  processId         The identifier of the process we would like to run the DLL from.
 * @param[in]  pInjectedDllAddr  Address of the DLL path's string in the remote process.
 * @param[in]  dllPath           path of the DLL to inject.
 *
 * @return     TRUE iff the DLL ran successfully
 */
BOOL runInjectedDLL(DWORD processId, LPVOID pInjectedDllAddr, const TCHAR* dllPath);

#endif // __INJECTOR_H__
