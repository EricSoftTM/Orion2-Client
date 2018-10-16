/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Eric
*
*/
#include "Orion.h"
#include "OrionHacks.h"

/* Initializing Memory Alterations */
BOOL bInitialized = FALSE;

/* CreateWindowExA hook used to rename the main window */
bool Hook_CreateWindowExA(bool bEnable) {
	static decltype(&CreateWindowExA) _CreateWindowExA = &CreateWindowExA;

	decltype(&CreateWindowExA) CreateWindowExA_Hook = [](DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) -> HWND {

		if (strstr(lpClassName, CLIENT_CLASS)) {
			//NotifyMessage("Orion2 has loaded all data successfully!\r\n\r\nPress OK to launch the game~", Orion::NotifyType::Information);
			lpWindowName = CLIENT_CLASS;
		}

		return _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	};

	return SetHook(bEnable, reinterpret_cast<void**>(&_CreateWindowExA), CreateWindowExA_Hook);
}

bool Hook_CreateMutexA(bool bEnable) {
	static decltype(&CreateMutexA) _CreateMutexA = &CreateMutexA;

	decltype(&CreateMutexA) CreateMutexA_Hook = [](LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) -> HANDLE {
		if (lpName && !lstrcmpiA(lpName, MUTLI_MUTEX)) {
			HANDLE hHandle = _CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);

			HANDLE hMaple;
			DuplicateHandle(GetCurrentProcess(), hHandle, 0, &hMaple, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
			CloseHandle(hMaple);
			return hHandle;
		}
		return _CreateMutexA(lpMutexAttributes, bInitialOwner, lpName);
	};

	return SetHook(bEnable, reinterpret_cast<void**>(&_CreateMutexA), CreateMutexA_Hook);
}

/* Best way to inject into client memory after class load: hook GetCurrentDirectoryA(). */
bool Hook_GetCurrentDirectoryA(bool bEnable) {
	static decltype(&GetCurrentDirectoryA) _GetCurrentDirectoryA = &GetCurrentDirectoryA;

	decltype(&GetCurrentDirectoryA) GetCurrentDirectoryA_Hook = [](DWORD nBufferLength, LPSTR lpBuffer) -> DWORD {
		/* Only initialize the memory alterations once.. */
		if (!bInitialized) {
			if (InitializeOrion2()) {
				bInitialized = TRUE;
			} else {
				return FALSE;
			}
		}

		return _GetCurrentDirectoryA(nBufferLength, lpBuffer);
	};

	return SetHook(bEnable, reinterpret_cast<void**>(&_GetCurrentDirectoryA), GetCurrentDirectoryA_Hook);
}

bool RedirectProcess() {
	//"C:\Nexon\Library\maplestory2\appdata\MapleStory2.exe" "127.0.0.1" 20001 --nxapp=nxl --lc=en-US
	LPTSTR sCmd = GetCommandLine();

	if (!strstr(sCmd, "nxapp")) {
		char sFileName[MAX_PATH];
		std::string strFileName = std::string(sFileName, GetModuleFileName(NULL, sFileName, MAX_PATH));
		std::string strCmd = std::string(sCmd);
		strCmd += " \"127.0.0.1\"";//IP
		strCmd += " 20001";//Port
		strCmd += " --nxapp=nxl";
		strCmd += " --lc=en-US";//Locale

		LPSTR lpProgramPath = _tcsdup(&strFileName[0]);
		LPSTR lpCommandLine = _tcsdup(TEXT(&strCmd[0]));

		PROCESS_INFORMATION p_info;
		STARTUPINFO s_info;
		memset(&s_info, 0, sizeof(s_info));
		memset(&p_info, 0, sizeof(p_info));
		s_info.cb = sizeof(s_info);
		if (CreateProcess(lpProgramPath, lpCommandLine, NULL, NULL, 0, 0, NULL, NULL, &s_info, &p_info)) {
			Sleep(2000);//Open in current window handle
			exit(0);//Exit this process so it doesn't redirect
			WaitForSingleObject(p_info.hProcess, INFINITE);
			CloseHandle(p_info.hProcess);
			CloseHandle(p_info.hThread);
			return true;
		}
		return false;
	}
	return true;
}

int LogReport(const char *sFormat, ...) {
	CHAR sBuff[4096] = { 0 };
	va_list arglist;

	va_start(arglist, sFormat);

	int v2 = wvsprintfA((LPSTR)(&sBuff), sFormat, arglist);

	OutputDebugStringA(sBuff);

	va_end(arglist);

	return v2;
}

LONG WINAPI DumpUnhandledException(LPEXCEPTION_POINTERS pExceptionInfo) {
	LogReport("==== DumpUnhandledException ==============================\r\n");

	auto v6 = pExceptionInfo->ExceptionRecord;
	LogReport("Fault Address : %08X\r\n", v6->ExceptionAddress);
	LogReport("Exception code: %08X\r\n", v6->ExceptionCode);

	auto v8 = pExceptionInfo->ContextRecord;
	LogReport("Registers:\r\n");
	LogReport(
		"EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n",
		v8->Eax, v8->Ebx, v8->Ecx,
		v8->Edx, v8->Esi, v8->Edi);
	LogReport("CS:EIP:%04X:%08X\r\n", v8->SegCs, v8->Eip);
	LogReport("SS:ESP:%04X:%08X  EBP:%08X\r\n", v8->SegSs, v8->Esp, v8->Ebp);
	LogReport("DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", v8->SegDs, v8->SegEs, v8->SegFs, v8->SegGs);
	LogReport("Flags:%08X\r\n", v8->EFlags);

	LogReport("\r\n");

	return EXCEPTION_EXECUTE_HANDLER;
}

void InitUnhandledExceptionFilter() {
	SetUnhandledExceptionFilter(DumpUnhandledException);
}

/* TODO: Confirm this works for MS2 (ZException object?) - Hydromoph */
//void* InitVectoredExceptionHandler() {
//	typedef const struct
//	{
//		unsigned int magic;
//		void* object;
//		_ThrowInfo* info;
//	} CxxThrowExceptionObject;
//
//	typedef struct ZException
//	{
//		HRESULT m_hr;
//	} ZException;
//
//	PVECTORED_EXCEPTION_HANDLER Handler = [](EXCEPTION_POINTERS* pExceptionInfo) -> long
//	{
//		if (pExceptionInfo->ExceptionRecord->ExceptionCode == 0xE06D7363) // C++ Exceptions
//		{
//			CxxThrowExceptionObject* exception_object = reinterpret_cast<CxxThrowExceptionObject*>(pExceptionInfo->ExceptionRecord->ExceptionInformation);
//
//			if (exception_object->magic == 0x19930520 && exception_object->info->pCatchableTypeArray->nCatchableTypes > 0)
//			{
//				typedef struct ZException
//				{
//					HRESULT m_hr;
//				} ZException;
//
//				ZException* exc = reinterpret_cast<ZException*>(exception_object->object);
//				printf("CMSException: %s - %08X\n", exception_object->info->pCatchableTypeArray->arrayOfCatchableTypes[0]->pType->name, exc->m_hr);
//			}
//		}
//		else if (pExceptionInfo->ExceptionRecord->ExceptionCode != STATUS_PRIVILEGED_INSTRUCTION && pExceptionInfo->ExceptionRecord->ExceptionCode != DBG_PRINTEXCEPTION_C)
//		{
//			if (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_HEAP_CORRUPTION || pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
//			{
//				if (pExceptionInfo->ExceptionRecord->ExceptionAddress != 0)
//				{
//					printf("RegException: %08X (%08X)\n", pExceptionInfo->ExceptionRecord->ExceptionCode, pExceptionInfo->ExceptionRecord->ExceptionAddress);
//
//					FILE* ff = fopen("C:\\exc.txt", "a+");
//
//					for (int i = 0; i < 0x400; i += 4)
//						fprintf(ff, "Esp+%02X = %08X\n", i, *(DWORD*)(pExceptionInfo->ContextRecord->Esp + i));
//
//					fclose(ff);
//				}
//			}
//			else
//				printf("RegException: %08X (%08X)\n", pExceptionInfo->ExceptionRecord->ExceptionCode, pExceptionInfo->ExceptionRecord->ExceptionAddress);
//		}
//
//		return EXCEPTION_CONTINUE_SEARCH;
//	};
//
//	return AddVectoredExceptionHandler(1, Handler);
//}

/* A defaulted MessageBox */
void NotifyMessage(const char* sText) {
	NotifyMessage(sText, Orion::NotifyType::None);
}

/* A MessageBox with information/warning/error/etc icons and dialogues */
void NotifyMessage(const char* sText, int nType) {
	MessageBoxA(NULL, sText, CLIENT_NAME, nType);
}

/* A MessageBox with the allowed use of arguments, used for misc debugging */
void NotifyDbgMessage(const char* sFormat, ...) {
	char* sText = new char[MAX_BUFFER];
	va_list args;
	va_start(args, sFormat);
	vsnprintf(sText, MAX_BUFFER - 1, sFormat, args);

	MessageBoxA(NULL, sText, CLIENT_NAME, NULL);

	va_end(args);
	delete[] sText;
}

/* A wide-character(W) version of NotifyDbgMessage */
void NotifyDbgMessageW(const wchar_t* sFormat, ...) {
	wchar_t* sText = new wchar_t[MAX_BUFFER];
	va_list args;
	va_start(args, sFormat);
	_vsnwprintf(sText, MAX_BUFFER - 1, sFormat, args);

	MessageBoxW(NULL, sText, NULL, NULL);

	va_end(args);
	delete[] sText;
}

/* Attach or detach a detour hook on to a given target */
BOOL SetHook(__in BOOL bInstall, __inout PVOID* ppvTarget, __in PVOID pvDetour) {
	if (DetourTransactionBegin() != NO_ERROR)
		return FALSE;

	if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
		if ((bInstall ? DetourAttach : DetourDetach)(ppvTarget, pvDetour) == NO_ERROR)
			if (DetourTransactionCommit() == NO_ERROR)
				return TRUE;

	DetourTransactionAbort();
	return FALSE;
}
