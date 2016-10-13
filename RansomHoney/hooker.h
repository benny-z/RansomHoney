#ifndef __HOOKER_H__
#define __HOOKER_H__

#include <Windows.h>
#include <stdio.h>
#include "..\Utils\DebugOutput.h"
#include "..\Utils\procUtils.h"

/**
 * A struct containing various data regarding a hook.
 * newFuncPtr - pointer to the new function, after the hook was applied.
 * origFuncPtr - pointer to the original function, before the hook was applied.
 * IATFuncAddr - address of the function in the IAT.
 * funcName - name of the function.
 * baseIATAddr - the base address of the IAT in the current process.
 * isHookSet - TRUE iff the hook is set.
 */
typedef struct {
	LPVOID newFuncPtr;
	LPVOID origFuncPtr;
	LPVOID IATFuncAddr;
	char funcName[MAX_PATH];
	LPVOID baseIATAddr;
	BOOL isHookSet;
} HookData;

/**
 * @brief      Sets the hook specified by the struct in the first argument
 *
 * @param      hookData  Data regarding the hook. Note that this pointer 
 *                       will be modified when the function is executed.
 * @param[in]  hModule   Handle to the open module.
 *
 * @return     TRUE iff all the WinApi calls were successful
 */
BOOL setHook(HookData* hookData, HMODULE hModule);

/**
 * @brief      Swaps the original function address in the IAT with 
 *             the new one (both specified in the HookData structure)
 *
 * @param      hookData  Data regarding the hook.
 *
 * @return     TRUE iff all the WinApi calls were successful
 */
BOOL patch(const HookData* hookData);

/**
 * @brief      Returns a pointer to the image import descriptor struct with information 
 *             about the current process. It will be further used to locate the IAT.
 *
 * @param      hModule  Handle to the current module.
 *
 * @return     Instance of the IMAGE_IMPORT_DESCRIPTOR struct
 */
PIMAGE_IMPORT_DESCRIPTOR getIATAddr(HMODULE hModule);

/**
 * @brief              Retrieves the address of the given function from the IAT of the 
 *                     current process.
 *
 * @param[in,out]      hookData  Data regarding the hook. Please note that the hookData 
 *                               variable will be modified.
 * @param              hModule   Handle to the current module.
 *
 * @return             TRUE iff all the WinApi calls were successful.
 */
BOOL getFuncIATAddr(HookData* hookData, HMODULE hModule);

/**
 * @brief      Removes a hook.
 *
 * @param[in]  hookData  Data regarding the hook.
 * @param[in]  hModule   Handle to the current module.
 *
 * @return     TRUE iff all the WinApi calls were successful.
 */
BOOL removeHook(const HookData* hookData, HMODULE hModule);

/**
 * @brief      Sets a hook on multiple files.
 *
 * @param      funcsToHook       List of HookData instances containing the necessary information
 *                               for the hook to succeed, the original function's name, the new function, etc.
 * @param[in]  hModule           Handle to the current module.
 * @param[in]  numOfFuncsToHook  The number of functions for he hook to be set on.
 */
VOID hookMultipleFuncs(HookData* funcsToHook, HMODULE hModule, DWORD numOfFuncsToHook);

#endif // __HOOKER_H__
