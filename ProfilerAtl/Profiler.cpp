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


int const bucketSize = 5000;
unsigned int* functionMap = new unsigned int[bucketSize];
unsigned int* fnMapPtr = functionMap;

CProfiler::CProfiler() 
{
	hLogFile = INVALID_HANDLE_VALUE;
	memset(functionMap, 0, bucketSize);
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

void __stdcall FunctionEnterGlobal(FunctionID functionID) {
	iCorProfilerCallback->LogLine("\r\nentered\r\n");
	mdToken mdToken = 0;
	IMetaDataImport* metadata = 0;
	HRESULT hr = iCorProfilerInfo->GetTokenAndMetaDataFromFunction(functionID, IID_IMetaDataImport, (LPUNKNOWN*)&metadata, &mdToken);
	if (FAILED(hr)) {
		return;
	}

	mdTypeDef typeDefToken = 0;
	wchar_t functionName[1000];
	ULONG outWideChar = 0;
	DWORD attr = 0;
	PCCOR_SIGNATURE sigBlob = NULL;
	ULONG sigBlobBytesCount = NULL;

	hr = metadata->GetMethodProps(mdToken,
		&typeDefToken, functionName,
		1000, &outWideChar,
		NULL, &sigBlob, &sigBlobBytesCount, NULL, NULL);

	if (FAILED(hr)) {
		return;
	}

	char* cFuncName = new char[1000];
	memset(cFuncName, 0, 1000);
	wcstombs(cFuncName, functionName, 1000);
	iCorProfilerCallback->Log("Function called: %s\r\n", cFuncName);
	metadata->Release();
}


void __declspec(naked) FunctionLeaveNaked(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO func, COR_PRF_FUNCTION_ARGUMENT_INFO* argumentInfo) {
	__asm {
		push ebp
		push EAX
		push EDX

		lea ebp, functionMap
		mov eax, funcId
		xor edx, edx
		div bucketSize
		// remainder in edx
		mov eax, edx
		add eax, fnMapPtr
		dec [eax]

		pop edx
		pop eax
		pop ebp
		ret 16
	}
}
void __declspec(naked) FunctionTailcallNaked(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO func) {
	__asm {
		ret 12
	}
}

void __stdcall FunctionEnterCriticalLevel(FunctionID  funcId, unsigned int amount) {
	cout << "function reaches critical level";
	cout << amount;
}

void __declspec(naked) FunctionEnterNaked(FunctionID funcId, UINT_PTR clientData, COR_PRF_FRAME_INFO func, COR_PRF_FUNCTION_ARGUMENT_INFO* argumentInfo) {
	__asm {
		push ebp
		push EAX
		push EDX

		mov ebp, esp
		mov eax, [ebp + 8]
		lea ebp, functionMap
		xor edx, edx
		div bucketSize
		// remainder in edx
		mov eax, edx
		add eax, fnMapPtr
		inc[eax]

		// compare
		cmp dword ptr [eax], 300
		jbe ignore

		push [eax]
		mov ebp, esp
		mov eax, [ebp + 8]
		push eax
		call FunctionEnterCriticalLevel

		ignore:

		pop edx
		pop eax
		pop ebp
		ret 16
	}
}



/*FunctionEnterNaked: __asm {
	push ebp
	mov ebp, esp

	mov EAX, [ebp + 8];
	push EAX
	call FunctionEnterGlobal

	pop ebp
	ret 16
}*/

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

	iCorProfilerInfo->SetEnterLeaveFunctionHooks2((FunctionEnter2*)&FunctionEnterNaked, (FunctionLeave2*)&FunctionLeaveNaked, (FunctionTailcall2*)&FunctionTailcallNaked);

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

void CProfiler::LogLine(wchar_t* pMsg) {
	Log(pMsg);
	Log("\r\n");
}

void CProfiler::LogLine(char* pMsg) {
	Log(pMsg);
	Log("\r\n");
}

void CProfiler::Log(wchar_t* pMsg)
{
	size_t size = wcslen(pMsg) + 1;
	size_t converted = 0;
	char* dest = new char[size];
	wcstombs_s(&converted, dest, size, pMsg, size);

	Log(dest);
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
	DWORD eventMask = (DWORD)(COR_PRF_MONITOR_ENTERLEAVE | COR_PRF_MONITOR_OBJECT_ALLOCATED | COR_PRF_ENABLE_OBJECT_ALLOCATED | COR_PRF_ENABLE_STACK_SNAPSHOT | COR_PRF_MONITOR_EXCEPTIONS);
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