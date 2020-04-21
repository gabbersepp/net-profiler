#include <assert.h>
#include "winnt.h"
#include "stdafx.h"
#include "Profiler.h"
#include <iostream>
#include "corprof.h";
#include <string>;
#include <vector>;
#include <windows.h>;

using namespace std;

const wchar_t* LogFileName = L"Profiler.log";
CComQIPtr<ICorProfilerInfo2> iCorProfilerInfo;
CProfiler* iCorProfilerCallback = NULL;

CProfiler::CProfiler() 
{
	hLogFile = INVALID_HANDLE_VALUE;
}

HRESULT CProfiler::FinalConstruct()
{
	CreateLogFile();
	return S_OK;
}

void CProfiler::FinalRelease()
{
	CloseLogFile();
}

HRESULT __stdcall CProfiler::Initialize(IUnknown *pICorProfilerInfoUnk)
{
	iCorProfilerCallback = this;
	LogLine("Initializing...");

    HRESULT hr = pICorProfilerInfoUnk->QueryInterface(IID_ICorProfilerInfo2, (LPVOID*)&iCorProfilerInfo);
	if (FAILED(hr)) {
		LogLine("Could not find ICorProfilerInfo");
		return E_FAIL;
	}

	hr = SetEventMask();
	if (FAILED(hr)) {
		LogLine("Error setting event mask");
	}

    return S_OK;
}

HRESULT __stdcall CProfiler::Shutdown()
{
	iCorProfilerCallback = NULL;

    return S_OK;
}

void CProfiler::CreateLogFile()
{
	::DeleteFile(LogFileName);
	hLogFile = CreateFile(LogFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

void CProfiler::CloseLogFile()
{
	if (hLogFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hLogFile);
		hLogFile = INVALID_HANDLE_VALUE;
	}
}

void CProfiler::LogLine(wchar_t* pMsg, ...) {
	va_list args;
	va_start(args, pMsg);
	Log(pMsg, args);
	Log("\r\n");
	va_end(args);
}

void CProfiler::LogLine(char* pMsg, ...) {
	va_list args;
	va_start(args, pMsg);
	Log(pMsg, args);
	Log("\r\n");
	va_end(args);
}

void CProfiler::Log(wchar_t* pMsg, ...)
{
	size_t size = wcslen(pMsg) + 1;
	size_t converted = 0;
	char* dest = new char[size];
	wcstombs_s(&converted, dest, size, pMsg, size);

	va_list args;
	va_start(args, pMsg);
	Log(dest, args);
	va_end(args);
}

void CProfiler::Log(char *pMsg, ...)
{
	CHAR buffer[4096]; DWORD dwWritten = 0;

	if(hLogFile != INVALID_HANDLE_VALUE)
	{
		va_list args;
		va_start( args, pMsg);
		vsprintf_s(buffer, 4096, pMsg, args);
		va_end( args );

		WriteFile(hLogFile, buffer, (DWORD)strlen(buffer), &dwWritten, NULL);
	}
}

HRESULT CProfiler::SetEventMask()
{
	DWORD eventMask = (DWORD)(COR_PRF_MONITOR_OBJECT_ALLOCATED | COR_PRF_ENABLE_OBJECT_ALLOCATED | COR_PRF_ENABLE_STACK_SNAPSHOT | COR_PRF_MONITOR_EXCEPTIONS);
	return iCorProfilerInfo->SetEventMask(eventMask);
}

HRESULT __stdcall CProfiler::ObjectAllocated(ObjectID objectID, ClassID classID)
{
	ModuleID moduleId = 0;
	mdTypeDef typeDef = 0;
	ClassID parentClassId = 0;
	ClassID typeArgs[2];
	ULONG32 numtypeArgs = 2;
	IMetaDataImport* metadata = 0;
	wchar_t className[1000];
	ULONG numberWideCharacters = 0;

	HRESULT hr = iCorProfilerInfo->GetClassIDInfo2(classID, &moduleId, &typeDef, &parentClassId, numtypeArgs, &numtypeArgs, typeArgs);
	if (FAILED(hr)) {
		//LogLine("Error retrioving class info");
		return E_FAIL;
	}

	hr = iCorProfilerInfo->GetModuleMetaData(moduleId, CorOpenFlags::ofRead, IID_IMetaDataImport, (LPUNKNOWN*)&metadata);
	if (FAILED(hr)) {
		//LogLine("Error retrioving metadata");
		return E_FAIL;
	}
	hr = metadata->GetTypeDefProps(typeDef, className, 1000, &numberWideCharacters, 0, 0);
	if (FAILED(hr)) {
		//LogLine("Error retrioving type def");
		return E_FAIL;
	}
	
	char* className2 = new char[wcslen(className) + 1];
	wcstombs(className2, className, wcslen(className) + 1);
	Log("Object allocated: %s\r\n", className2);

	//if (wcscmp(className, L"ErrorThrowingApp.Test") == 0) {
		GetStacktrace();
	//}

	metadata->Release();
	return S_OK;
}

void CProfiler::GetStacktrace() {
	LogLine("\r\nStacktrace:");

	WOW64_CONTEXT context = { 0 };
	context.ContextFlags = WOW64_CONTEXT_FULL;
	std::vector<FunctionID>* functions = new std::vector<FunctionID>();
	// wenn context != 0 -> asynchron?
	// https://docs.microsoft.com/de-de/dotnet/framework/unmanaged-api/profiling/icorprofilerinfo2-dostacksnapshot-method#synchronous-stack-walk
	iCorProfilerInfo->DoStackSnapshot(NULL, &MyDoStackSnapshotCallback, COR_PRF_SNAPSHOT_REGISTER_CONTEXT, &functions, 0, 0);// (BYTE*)&context, sizeof(WOW64_CONTEXT));

	LogLine("\r\n");
}


HRESULT __stdcall CProfiler::ExceptionThrown(ObjectID thrownObjectID)
{
	LogLine("exception was thrown\r\n");

	return S_OK;
}

HRESULT __stdcall CProfiler::ExceptionCatcherEnter(FunctionID functionID, ObjectID objectID) {
	LogLine("yeah some nice guy catched that exception. Well done!");

	return S_OK;
}

HRESULT __stdcall CProfiler::MyDoStackSnapshotCallback(
	FunctionID funcId,
	UINT_PTR ip,
	COR_PRF_FRAME_INFO frameInfo,
	ULONG32 contextSize,
	BYTE context[],
	void* clientData
)
{
	if (funcId == 0) {
		// unmanaged call
		return 0;
	}
	ClassID classId = 0;
	mdToken pToken = 0;

	IMetaDataImport* metadata = 0;
	mdToken methodToken = 0;
	HRESULT hr = iCorProfilerInfo->GetTokenAndMetaDataFromFunction(funcId, IID_IMetaDataImport, (LPUNKNOWN*)&metadata, &methodToken);
	if (FAILED(hr)) {
		//iCorProfilerCallback->LogLine("Error retrioving metadata");
		return E_FAIL;
	}

	mdTypeDef typeDefToken = 0;
	wchar_t functionName[1000];
	ULONG outWideChar = 0;
	DWORD attr = 0;
	PCCOR_SIGNATURE sigBlob = NULL;
	ULONG sigBlobBytesCount = NULL;

	hr = metadata->GetMethodProps(methodToken,
		&typeDefToken, functionName,
		1000, &outWideChar,
		NULL, &sigBlob, &sigBlobBytesCount, NULL, NULL);

	if (FAILED(hr)) {
		//iCorProfilerCallback->LogLine("Error retrioving type def");
		return E_FAIL;
	}


	wchar_t typeName[1000];

	hr = metadata->GetTypeDefProps(typeDefToken, typeName, 1000, &outWideChar, NULL, NULL);

	iCorProfilerCallback->Log(typeName);
	iCorProfilerCallback->Log(".");
	iCorProfilerCallback->Log(functionName);
	iCorProfilerCallback->Log("\r\n");

	metadata->Release();

	return 0;
}