#ifndef __COMMON_FILES_LIST_H__
#define __COMMON_FILES_LIST_H__

#include <Windows.h>

#define NUM_OF_FILES (4)
#define NUM_OF_HIDE_FROM_PROCS (2)

/**
 * @return     List of names of the files that should not be visible to the user. 
 *             Accessing these files will imply suspicious behavior, and would be
 *             treated accordingly, as specified in the fileWatcher DLL.
 */
const TCHAR** getFilesList();

/**
 * @return     List of names of the processes the files should be hidden from.
 */
const TCHAR** getProcsToHideFrom();

#endif // __COMMON_FILES_LIST_H__