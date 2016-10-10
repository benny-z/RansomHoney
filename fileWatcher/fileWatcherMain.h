#ifndef __FILE_WATCHER_MAIN_H__
#define __FILE_WATCHER_MAIN_H__

#include "..\Utils\DebugOutput.h"
#include "..\RansomHoney\hooker.h"
#include "..\RansomHoney\RansomHoney.h"

/**
 * @brief      Creates or opens a file or I/O device. 
 * 
 * @note       For the full documentation, please see the MSDN article for the CreateFile function:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
 *
 * @param[in]  lpFileName             The name of the file or device to be created or opened. 
 * @param[in]  dwDesiredAccess        The requested access to the file or device, which can be 
 *                                    summarized as read, write, both or neither zero).
 * @param[in]  dwShareMode            The requested sharing mode of the file or device, which 
 *                                    can be read, write, both, delete, all of these, or none.
 * @param[in]  lpSecurityAttributes   A pointer to a SECURITY_ATTRIBUTES structure.
 * @param[in]  dwCreationDisposition  An action to take on a file or device that exists or does not exist.
 * @param[in]  dwFlagsAndAttributes   The file or device attributes and flags.
 * @param[in]  hTemplateFile          A valid handle to a template file with the GENERIC_READ access right. 
 *
 * @return     If the function succeeds, the return value is an open handle to the specified file, device, 
 * named pipe, or mail slot. Or, INVALID_HANDLE_VALUE otherwise.
 */
HANDLE WINAPI myCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

/**
 * @brief      Creates or opens a file or I/O device. 
 * 
 * @note       For the full documentation, please see the MSDN article for the CreateFile function:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
 *
 * @param[in]  lpFileName             The name of the file or device to be created or opened. 
 * @param[in]  dwDesiredAccess        The requested access to the file or device, which can be 
 *                                    summarized as read, write, both or neither zero).
 * @param[in]  dwShareMode            The requested sharing mode of the file or device, which 
 *                                    can be read, write, both, delete, all of these, or none.
 * @param[in]  lpSecurityAttributes   A pointer to a SECURITY_ATTRIBUTES structure.
 * @param[in]  dwCreationDisposition  An action to take on a file or device that exists or does not exist.
 * @param[in]  dwFlagsAndAttributes   The file or device attributes and flags.
 * @param[in]  hTemplateFile          A valid handle to a template file with the GENERIC_READ access right. 
 *
 * @return     If the function succeeds, the return value is an open handle to the specified file, device, 
 * named pipe, or mail slot. Or, INVALID_HANDLE_VALUE otherwise.
 */
HANDLE WINAPI myCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);

#endif // __FILE_WATCHER_MAIN_H__