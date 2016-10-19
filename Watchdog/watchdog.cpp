/************************************************************************
THIS FILE IS BASED ON
https ://msdn.microsoft.com/en-us/library/aa390425(VS.85).aspx
WILL BE RE - WRITTEN ASAP!
************************************************************************/
#include "watchdog.h"

using namespace std;

BOOL startWtchdg(injectToAndRunNewProcPtr injectToAndRunNewProc, HANDLE g_serviceStop){
	debugOutput(L"Watchdog started");
	HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres)) {
		debugOutputNum(L"Error in startWtchdg. CoInitializeEx failed (%d)", GetLastError());
		return FALSE;
	}

	hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM negotiates service
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);

	if (FAILED(hres)) {
		debugOutputNum(L"Error in startWtchdg. CoInitializeSecurity failed (%d)", GetLastError());
		CoUninitialize();
		return FALSE;
	}

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hres)) {
		debugOutputNum(L"Error in startWtchdg. CoCreateInstance failed (%d)", GetLastError());
		CoUninitialize();
		return FALSE;
	}

	IWbemServices *pSvc = NULL;
	hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

	if (FAILED(hres)) {
		debugOutputNum(L"Error in startWtchdg. ConnectServer failed (%d)", GetLastError());
		pLoc->Release();
		CoUninitialize();
		return FALSE;
	}

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx 
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx 
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);

	if (FAILED(hres)) {
		debugOutputNum(L"Error in startWtchdg. CoSetProxyBlanket failed (%d)", GetLastError());
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return FALSE;
	}

	IUnsecuredApartment* pUnsecApp = NULL;

	hres = CoCreateInstance(CLSID_UnsecuredApartment, NULL,
		CLSCTX_LOCAL_SERVER, IID_IUnsecuredApartment,
		(void**)&pUnsecApp);

	EventSink* pSink = new EventSink(injectToAndRunNewProc);
	pSink->AddRef();

	IUnknown* pStubUnk = NULL;
	pUnsecApp->CreateObjectStub(pSink, &pStubUnk);

	IWbemObjectSink* pStubSink = NULL;
	pStubUnk->QueryInterface(IID_IWbemObjectSink, (void **)&pStubSink);

	hres = pSvc->ExecNotificationQueryAsync(
		_bstr_t("WQL"),
		_bstr_t("SELECT * "
			"FROM __InstanceCreationEvent WITHIN 1 "
			"WHERE TargetInstance ISA 'Win32_Process'"),
		WBEM_FLAG_SEND_STATUS,
		NULL,
		pStubSink);

	if (FAILED(hres)) {
		debugOutputNum(L"Error in startWtchdg. ExecNotificationQueryAsync failed (%d)", GetLastError());
		pSvc->Release();
		pLoc->Release();
		pUnsecApp->Release();
		pStubUnk->Release();
		pSink->Release();
		pStubSink->Release();
		CoUninitialize();
		return FALSE;
	}

	WaitForSingleObject(g_serviceStop, INFINITE);

	hres = pSvc->CancelAsyncCall(pStubSink);
	pSvc->Release();
	pLoc->Release();
	pUnsecApp->Release();
	pStubUnk->Release();
	pSink->Release();
	pStubSink->Release();
	CoUninitialize();

	return TRUE;
}

ULONG EventSink::AddRef() {
	return InterlockedIncrement(&m_lRef);
}

ULONG EventSink::Release() {
	LONG lRef = InterlockedDecrement(&m_lRef);
	if (lRef == 0)
		delete this;
	return lRef;
}

HRESULT EventSink::QueryInterface(REFIID riid, void** ppv) {
	if (riid == IID_IUnknown || riid == IID_IWbemObjectSink) {
		*ppv = (IWbemObjectSink *) this;
		AddRef();
		return WBEM_S_NO_ERROR;
	}
	else {
		return E_NOINTERFACE;
	}
}

HRESULT EventSink::Indicate(long lObjectCount, IWbemClassObject **apObjArray) {
	HRESULT hr = S_OK;
	_variant_t vtProp;

	for (int i = 0; i < lObjectCount; i++) {
		hr = apObjArray[i]->Get(_bstr_t(L"TargetInstance"), 0, &vtProp, 0, 0);
		if (SUCCEEDED(hr)) {
			IUnknown* str = vtProp;
			hr = str->QueryInterface(IID_IWbemClassObject, reinterpret_cast< void** >(&apObjArray[i]));
			if (SUCCEEDED(hr)) {
				_variant_t cn;
				hr = apObjArray[i]->Get(L"Handle", 0, &cn, NULL, NULL);
				if (SUCCEEDED(hr)) {
					// Making sure it's the correct variable type (vt)
					if ((cn.vt != VT_NULL) && (cn.vt != VT_EMPTY) && !(cn.vt & VT_ARRAY)) {
						DWORD processId = _wtoi(cn.bstrVal);
						m_injectToAndRunNewProc(processId);
					}
				}
				VariantClear(&cn);
			}
		}
		VariantClear(&vtProp);
	}

	return WBEM_S_NO_ERROR;
}

HRESULT EventSink::SetStatus(
	/* [in] */ LONG lFlags,
	/* [in] */ HRESULT hResult,
	/* [in] */ BSTR strParam,
	/* [in] */ IWbemClassObject __RPC_FAR *pObjParam) {
	if (lFlags == WBEM_STATUS_COMPLETE) {
		printf("Call complete. hResult = 0x%X\n", hResult);
	}
	else if (lFlags == WBEM_STATUS_PROGRESS) {
		printf("Call in progress.\n");
	}

	return WBEM_S_NO_ERROR;
}    // end of EventSink.cpp