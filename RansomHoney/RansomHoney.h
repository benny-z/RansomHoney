#ifndef __RANSOM_HONEY_H__
#define __RANSOM_HONEY_H__

#include <Windows.h>
#include <Shlwapi.h>
#include "injector.h"
#include "..\RansomHoney\commonFilesList.h"

/**
 * @brief      Creates the files (if they do not already exist) that are specified by the 
 *             global g_filesList in commonFilesList.c. Accessing these files will imply 
 *             suspicious behavior, and would be treated accordingly, as specified in the 
 *             fileWatcher DLL.
 * 
 * @return     TRUE iff all the files were created successfully.
 */
BOOL createFiles();

/**
 * @brief      Injects the fileWatcher DLL to all of the processes.
 *
 * @return     TRUE iff fileWatcher was successfully injected. 
 *             Please note that a positive result from this function doesn't necessarily mean
 *             that fileWatcher.dll was injected to ALL of the processes.
 */
BOOL initFileWatcher();

/**
 * @brief      Injects the fileHider DLL to all of the processes and by that, hides the file from
 *             the user.
 *
 * @return     TRUE iff fileHider was successfully injected.
 *             Please note that a positive result from this function doesn't necessarily mean
 *             that fileHider.dll was injected to ALL of the processes.
 */
BOOL hideFiles();

/**
 * @brief      Returns TRUE iff the specified file should not be visible to the user.
 *
 * @param[in]  filename  The name of the file.
 *
 */
BOOL isFileHiddenA(const char* filename);

/**
 * @brief      Returns TRUE iff the specified file should not be visible to the user.
 *
 * @param[in]  filename  The name of the file.
 *
 */
BOOL isFileHiddenW(const wchar_t* curFilename);

#endif // __RANSOM_HONEY_H__