/************************************************************************
THIS FILE IS BASED ON 
https://msdn.microsoft.com/en-us/library/aa390425(VS.85).aspx
WILL BE RE-WRITTEN ASAP!
************************************************************************/
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#define _WIN32_DCOM

#include <Windows.h>
#include "..\Utils\DebugOutput.h"

#ifdef __cplusplus
#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif // __cplusplus

typedef BOOL (*injectToAndRunNewProcPtr)(DWORD processId);
EXTERNC BOOL startWtchdg(injectToAndRunNewProcPtr injectToAndRunNewProc, HANDLE g_serviceStop);

#ifdef __cplusplus
class EventSink : public IWbemObjectSink {
	LONG m_lRef;
	bool bDone;
	injectToAndRunNewProcPtr m_injectToAndRunNewProc;
public:
	EventSink(injectToAndRunNewProcPtr injectToAndRunNewProc) { 
		m_lRef = 0; m_injectToAndRunNewProc = injectToAndRunNewProc; 
	}
	~EventSink() { bDone = true; }

	virtual ULONG STDMETHODCALLTYPE AddRef();
	virtual ULONG STDMETHODCALLTYPE Release();
	virtual HRESULT
		STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);

	virtual HRESULT STDMETHODCALLTYPE Indicate(
		LONG lObjectCount,
		IWbemClassObject __RPC_FAR *__RPC_FAR *apObjArray
	);

	virtual HRESULT STDMETHODCALLTYPE SetStatus(
		/* [in] */ LONG lFlags,
		/* [in] */ HRESULT hResult,
		/* [in] */ BSTR strParam,
		/* [in] */ IWbemClassObject __RPC_FAR *pObjParam
	);
};
#endif // __cplusplus

#endif    // end of EventSink.h