#include "hooker.h"

#define ResolveRVA(base,rva) (( (BYTE*)base) + rva)

BOOL setHook(HookData* hookData, HMODULE hModule) {
	LPVOID newFuncAddr = hookData->newFuncPtr;
	if (NULL == hModule) {
		debugOutputNum(L"Hooker: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	BOOL IATAddrSuccess = getFuncIATAddr(hookData, hModule);
	if (FALSE == IATAddrSuccess) {
		return FALSE;
	}
	if (FALSE == patch(hookData)) {
		debugOutputNum(L"Hooker: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	debugOutput(L"Hooker: patched successfully");
	return TRUE;
}

BOOL patch(const HookData* hookData) {
	if (NULL == hookData->origFuncPtr) {
		debugOutputNum(L"Hooker: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	DWORD oldRights = -1;
	DWORD newRights = PAGE_READWRITE;

	HMODULE hProc = GetCurrentProcess();
	if (0 == VirtualProtect(hookData->IATFuncAddr, sizeof(LPVOID), newRights, &oldRights)) {
		debugOutputNum(L"Hooker: Error in patch: 0x%08lx", GetLastError());
		return FALSE;
	}

	if (0 != memcpy_s(hookData->IATFuncAddr, sizeof(SIZE_T), &hookData->newFuncPtr, sizeof(SIZE_T))) {
		debugOutputNum(L"Hooker: Error in patch: 0x%08lx", GetLastError());
		return FALSE;
	}

	if (0 == VirtualProtect(hookData->IATFuncAddr, sizeof(LPVOID), oldRights, &newRights)) {
		debugOutputNum(L"Hooker: Error in patch: 0x%08lx", GetLastError());
		return FALSE;
	}
	return TRUE;
}

PIMAGE_IMPORT_DESCRIPTOR getIATAddr(HMODULE hModule) {
	if (NULL == hModule) {
		debugOutputNum(L"Hooker: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
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

BOOL initBaseIATAddr(HookData* hookData, HMODULE hModule) {
	if (NULL == hookData->baseIATAddr) {
		PIMAGE_IMPORT_DESCRIPTOR pImgImportDesc = getIATAddr(hModule);
		if (NULL == pImgImportDesc) {
			debugOutputNum(L"Error in initBaseIATAddr. getIATAddr failed (0x%08lx)\n", GetLastError());
			return FALSE;
		}
		hookData->baseIATAddr = pImgImportDesc;
	}
	return TRUE;
}

BOOL getFuncIATAddr(HookData* hookData, HMODULE hModule) {
	if (NULL == hModule) {
		debugOutput(L"Error in getFuncIATAddr. hModule is NULL\n");
		return FALSE;
	}
	if (!initBaseIATAddr(hookData, hModule)) {
		return FALSE;
	}

	for (PIMAGE_IMPORT_DESCRIPTOR pImgImportDesc = hookData->baseIATAddr; 
		pImgImportDesc->Characteristics != 0; 
		++pImgImportDesc) {
		PIMAGE_THUNK_DATA pOriginalThunk = (PIMAGE_THUNK_DATA)ResolveRVA(hModule, pImgImportDesc->OriginalFirstThunk);
		if (!pOriginalThunk) {
			continue;
		}

		for (PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)ResolveRVA(hModule, pImgImportDesc->FirstThunk);
			0 != pOriginalThunk->u1.Function; 
			pOriginalThunk++, pThunk++) {
			if (IMAGE_SNAP_BY_ORDINAL(pOriginalThunk->u1.Ordinal)) {
				continue;
			}
			PIMAGE_IMPORT_BY_NAME pImImport = (PIMAGE_IMPORT_BY_NAME)ResolveRVA(hModule, pOriginalThunk->u1.AddressOfData);
			if (0 == _stricmp(hookData->funcName, pImImport->Name)) {
				hookData->IATFuncAddr = &(pThunk->u1.Function);
				return TRUE;
			}
		}
	}
	//debugOutputStr(L"Hooker: No match found for %s", hookData->funcName);
	return FALSE;
}

BOOL removeHook(const HookData* hookData, HMODULE hModule) {
	if (NULL == hModule) {
		debugOutputNum(L"Hooker: Error in getFuncIATAddr: 0x%08lx\n", GetLastError());
		return FALSE;
	}
	if (NULL == hookData->origFuncPtr) {
		debugOutputNum(L"Hooker: Error in removeHook: 0x%08lx\n", GetLastError());
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
		//debugOutputNum(L"Hooker: Trying to hook function number 0x%08lx", i);
		if (FALSE == setHook((funcsToHook+i), hModule)) {
			//debugOutputNum(L"Hooker: failed to set hook. (0x%08lx)", GetLastError());
		}
		else {
			debugOutputNum(L"Hooker: hook on 0x%08lx set successfully", i);
			(funcsToHook + i)->isHookSet = TRUE;
		}
	}
}