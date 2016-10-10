#ifndef __FILE_HIDER_MAIN_H__
#define __FILE_HIDER_MAIN_H__

#include "..\Utils\DebugOutput.h"
#include "..\RansomHoney\hooker.h"
#include "..\RansomHoney\RansomHoney.h"
#include "..\RansomHoney\commonFilesList.h"

/**
 * @brief      Searches a directory for a file or subdirectory with a name that matches a specific name.
 *
 * @note       For the full documentation, please see the MSDN article for the FindFirstFile function:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/aa364418(v=vs.85).aspx
 * 
 * @param[in]  lpFileName      The directory or path, and the file name. 
 * @param[in]  lpFindFileData  A pointer to the WIN32_FIND_DATA structure that receives information about 
 *                             a found file or directory.
 *
 * @return     If the function succeeds, the return value is a search handle used in a
 *             subsequent call to FindNextFile or FindClose, and the lpFindFileData parameter
 *             contains information about the first file or directory found. 
 *             Or INVALID_HANDLE_VALUE otherwise.
 */
HANDLE WINAPI myFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData);

/**
 * @brief      Searches a directory for a file or subdirectory with a name that matches a specific name.
 *
 * @note       For the full documentation, please see the MSDN article for the FindFirstFile function:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/aa364418(v=vs.85).aspx
 * 
 * @param[in]  lpFileName      The directory or path, and the file name. 
 * @param[in]  lpFindFileData  A pointer to the WIN32_FIND_DATA structure that receives information about 
 *                             a found file or directory.
 *
 * @return     If the function succeeds, the return value is a search handle used in a
 *             subsequent call to FindNextFile or FindClose, and the lpFindFileData parameter
 *             contains information about the first file or directory found. 
 *             Or INVALID_HANDLE_VALUE otherwise.
 */
HANDLE WINAPI myFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData);

/**
 * @brief      Continues a file search from a previous call to the FindFirstFile, FindFirstFileEx, functions.
 *
 * @note       For the full documentation, please see the MSDN article for the FindNextFile function:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/aa364428(v=vs.85).aspx
 * 
 * @param[in]  hFindFile       The search handle returned by a previous call to the FindFirstFile function.
 * @param[in]  lpFindFileData  A pointer to the WIN32_FIND_DATA structure that receives information about the found file or subdirectory.
 *
 * @return     If the function succeeds, the return value is nonzero and the lpFindFileData parameter contains information about the next 
 *             file or directory found.
 */
BOOL WINAPI myFindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData);

/**
 * @brief      Continues a file search from a previous call to the FindFirstFile, FindFirstFileEx, functions.
 *
 * @note       For the full documentation, please see the MSDN article for the FindNextFile function:
 *             https://msdn.microsoft.com/en-us/library/windows/desktop/aa364428(v=vs.85).aspx
 * 
 * @param[in]  hFindFile       The search handle returned by a previous call to the FindFirstFile function.
 * @param[in]  lpFindFileData  A pointer to the WIN32_FIND_DATA structure that receives information about the found file or subdirectory.
 *
 * @return     If the function succeeds, the return value is nonzero and the lpFindFileData parameter contains information about the next 
 *             file or directory found.
 */
BOOL WINAPI myFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData);

#endif // __FILE_HIDER_MAIN_H__