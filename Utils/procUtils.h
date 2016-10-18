#ifndef __PROC_UTILS_H__
#define __PROC_UTILS_H__

#include <Windows.h>
#include <tchar.h>
#include <psapi.h>
#include <Winternl.h>
#include <TlHelp32.h>
#include "DebugOutput.h"

#define MAX_NUMBER_OF_PROCS (1024)
#define INVALID_SIZE (-1)

typedef enum {
	ARCH_64 = 64,
	ARCH_32 = 32
} architecture;

BOOL getFilenameByHandle(HANDLE hFile, LPTSTR lpszFilePath);

BOOL getCurProcArchitecture();

/**
 * @brief      Retrieves the Ids of all the processes currently running in the system
 *
 * @param[out]  procIds        List of ids of all the processes. Should be initialized
 *                             with zeros so that WinApi will be happy.
 * @param[in]   sizeOfProcIds  The size of procIds list
 * @param[out]  numOfProcs     The real number of processes running in the system i.e., the
 *                             number of items in procIds that have a meaningful value
 *
 * @return     TRUE iff all the API calls were successful
 */
BOOL getAllProcsIds(DWORD procIds[], DWORD sizeOfProcIds, DWORD* numOfProcs);

/**
 * @brief      Retrieves the identifier of a process with the given name
 *
 * @param  procName  Name of the process
 * @note   In case there are more than a single process with the given name, the 
 *         first one will be returned i.e., the one with the lowest id.
 *
 * @return     Identifier of the process with the given name
 */
DWORD getPorcIdByName(const wchar_t* procName);

/**
 * @brief      Determines whether a given process is suspended or not.
 *
 * @param[in]  procId  The process identifier
 *
 * @note       Not yet implemented 
 * 
 * @return     TRUE iff the process is suspended.
 */
BOOL isProcSuspended(DWORD procId);

architecture getProcArchitecture(DWORD procId, HANDLE hProc);

#endif // __PROC_UTILS_H__