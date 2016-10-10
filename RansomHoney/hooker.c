#include "hooker.h"

BOOL setHook(HookData* hookData, HMODULE hModule) {
	LPVOID newFuncAddr = hookData->newFuncPtr;
	debugOutput(L"HookerDLL: In setHook");
	if (NULL == hModule) {
		debugOutputNum(L"HookerDLL: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	BOOL IATAddrSuccess = getFuncIATAddr(hookData, hModule);
	if (FALSE == IATAddrSuccess) {
		return FALSE;
	}
	debugOutput(L"HookerDLL: got the original function IAT address");
	if (FALSE == patch(hookData)) {
		debugOutputNum(L"HookerDLL: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	debugOutput(L"HookerDLL: patched successfully");
	return TRUE;
}

BOOL patch(const HookData* hookData) {
	if (NULL == hookData->origFuncPtr) {
		debugOutputNum(L"HookerDLL: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	DWORD oldRights = -1;
	DWORD newRights = PAGE_READWRITE;

	HMODULE hProc = GetCurrentProcess();
	if (0 == VirtualProtect(hookData->IATFuncAddr, sizeof(LPVOID), newRights, &oldRights)) {
		debugOutputNum(L"HookerDLL: Error in patch: 0x%08lx", GetLastError());
		return FALSE;
	}

	if (0 != memcpy_s(hookData->IATFuncAddr, sizeof(SIZE_T), &hookData->newFuncPtr, sizeof(SIZE_T))) {
		debugOutputNum(L"HookerDLL: Error in patch: 0x%08lx", GetLastError());
		return FALSE;
	}

	if (0 == VirtualProtect(hookData->IATFuncAddr, sizeof(LPVOID), oldRights, &newRights)) {
		debugOutputNum(L"HookerDLL: Error in patch: 0x%08lx", GetLastError());
		return FALSE;
	}
	return TRUE;
}

PIMAGE_IMPORT_DESCRIPTOR getIATAddr(HMODULE hModule) {
	if (NULL == hModule) {
		debugOutputNum(L"HookerDLL: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return NULL;
	}
	PIMAGE_DOS_HEADER pImgDosHeaders = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pImgNTHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)pImgDosHeaders + pImgDosHeaders->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR pImgImportDesc =
		(PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)pImgDosHeaders + pImgNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	if (pImgDosHeaders->e_magic != IMAGE_DOS_SIGNATURE) {
		debugOutput(L"e_magic is no valid DOS signature\n");
		return NULL;
	}
	return pImgImportDesc;
}

BOOL getFuncIATAddr(HookData* hookData, HMODULE hModule) {
	if (NULL == hModule) {
		debugOutputNum(L"HookerDLL: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	PIMAGE_IMPORT_DESCRIPTOR pImgImportDesc = NULL;
	if (hookData->baseIATAddr) {
		pImgImportDesc = hookData->baseIATAddr;
	} else {
		PIMAGE_IMPORT_DESCRIPTOR pImgImportDesc = getIATAddr(hModule);
		if (NULL == pImgImportDesc) {
			debugOutputNum(L"HookerDLL: Error in getFuncIATAddr, failed to get image import descriptor: 0x%08lx\n", GetLastError());
			return FALSE;
		}
	}
	debugOutputStr(L"HookerDLL: Looking for the function: %s", hookData->funcName);

	for (; pImgImportDesc->FirstThunk; ++pImgImportDesc) {
		IMAGE_THUNK_DATA* thunkData = (IMAGE_THUNK_DATA*)((BYTE*)hModule + pImgImportDesc->OriginalFirstThunk);
		for (int i = 0; thunkData->u1.Function; ++i, ++thunkData) {
			char* funcName = (char*)((BYTE*)hModule + (DWORD)thunkData->u1.AddressOfData + 2);
			if (0 == strcmp(funcName, hookData->funcName)) {
				LPVOID funcAddr = (SIZE_T*)((BYTE*)hModule + pImgImportDesc->FirstThunk) + i;
				hookData->IATFuncAddr = funcAddr;
				return TRUE;
			}
		}
	}

	debugOutputStr(L"HookerDLL: No match found for %s", hookData->funcName);
	return FALSE;
}

BOOL removeHook(const HookData* hookData, HMODULE hModule) {
	if (NULL == hModule) {
		debugOutputNum(L"HookerDLL: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	if (NULL == hookData->origFuncPtr) {
		debugOutputNum(L"HookerDLL: Error in removeHook: 0x%08lx\n", GetLastError());
		return FALSE;
	}

	// Swapping the original function address and the new one's w/o harming the original HookData
	HookData reverseSrcDstFuncs = *hookData;
	reverseSrcDstFuncs.newFuncPtr = hookData->origFuncPtr;
	reverseSrcDstFuncs.origFuncPtr = hookData->newFuncPtr;

	return setHook(&reverseSrcDstFuncs, hModule);
}

VOID hookMultipleFuncs(HookData* funcsToHook, HMODULE hModule, DWORD numOfFuncsToHook) {
	for (DWORD i = 0; i < numOfFuncsToHook; ++i) {
		debugOutputNum(L"HookerDLL: Trying to hook function number 0x%08lx", i);
		if (FALSE == setHook((funcsToHook+i), hModule)) {
			debugOutputNum(L"HookerDLL: failed to set hook. (0x%08lx)", GetLastError());
		}
		else {
			debugOutputNum(L"HookerDLL: hook on 0x%08lx set successfully", i);
			(funcsToHook + i)->isHookSet = TRUE;
		}
	}
}