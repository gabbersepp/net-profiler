#pragma once

#include "resource.h"
#include "ProfilerAtl.h"
#include "CorProfilerCallbackDefaultImpl.h"
#include "EventLogger.h";

class ATL_NO_VTABLE CProfiler :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CProfiler, &CLSID_Profiler>,
	public CorProfilerCallbackDefaultImpl
{
public:
	CProfiler();

	DECLARE_REGISTRY_RESOURCEID(IDR_PROFILER)
	BEGIN_COM_MAP(CProfiler)
		COM_INTERFACE_ENTRY(ICorProfilerCallback)
		COM_INTERFACE_ENTRY(ICorProfilerCallback2)
	END_COM_MAP()
	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct();
	void FinalRelease();

    STDMETHOD(Initialize)(IUnknown *pICorProfilerInfoUnk);
    STDMETHOD(Shutdown)();


	HRESULT __stdcall ObjectAllocated(ObjectID objectID, ClassID classID);
	HRESULT __stdcall ExceptionThrown(ObjectID thrownObjectID);
	HRESULT __stdcall ExceptionCatcherEnter(FunctionID functionID, ObjectID objectID);
	static HRESULT __stdcall MyDoStackSnapshotCallback(
		FunctionID funcId,
		UINT_PTR ip,
		COR_PRF_FRAME_INFO frameInfo,
		ULONG32 contextSize,
		BYTE context[],
		void* clientData);

    void LogLine(char* pszFmtString );
	void LogLine(wchar_t* pMsg);
	void Log(wchar_t* pMsg);
	void Log(char* pMsg, ...);

private:

	HANDLE hLogFile;
	EventLogger* eventLogger;

	HRESULT SetEventMask();
	void CreateLogFile();
	void CloseLogFile();

	void GetStacktrace();
};

OBJECT_ENTRY_AUTO(__uuidof(Profiler), CProfiler)
