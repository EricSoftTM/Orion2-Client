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
			lpWindowName = CLIENT_NAME;
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

			if (hHandle) {
				HANDLE hMaple;
				DuplicateHandle(GetCurrentProcess(), hHandle, 0, &hMaple, 0, FALSE, DUPLICATE_CLOSE_SOURCE);
				CloseHandle(hMaple);
			}
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
	//"C:\Nexon\Library\maplestory2\appdata\x64\MapleStory2.exe" 127.0.0.1 30000 -ip -port --nxapp=nxl --lc=EN
	LPTSTR sCmd = GetCommandLine();

	if (!strstr(sCmd, "nxapp")) {
		char strFileName[MAX_PATH];
		char strCmd[MAX_BUFFER];
		GetModuleFileName(NULL, strFileName, MAX_PATH);
		strcpy(strCmd, sCmd);

		char sArgs[SCHAR_MAX];
		sprintf(sArgs, "%s %d -ip -port --nxapp=nxl --lc=%s", CLIENT_IP, CLIENT_PORT, CLIENT_LOCALE);
		strcat(strCmd, sArgs);

		PROCESS_INFORMATION p_info;
		STARTUPINFO s_info;
		memset(&s_info, 0, sizeof(s_info));
		memset(&p_info, 0, sizeof(p_info));
		s_info.cb = sizeof(s_info);
		if (CreateProcess(strFileName, strCmd, NULL, NULL, 0, 0, NULL, NULL, &s_info, &p_info)) {
			Sleep(100);//Open in current window handle
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

/* Orion's console logging function used in debug-mode only */
void Log(const char* sFormat, ...) {
#if DEBUG_MODE
	// Construct a formatted string buffer
	char* sText = new char[MAX_BUFFER];
	va_list aArgs;
	va_start(aArgs, sFormat);
	vsnprintf(sText, MAX_BUFFER - 1, sFormat, aArgs);
	// Concat a new line since we're printf-ing
	strcat(sText, "\n");
	// Print the formatted string to console
	printf(sText);
	// Cleanup memory
	va_end(aArgs);
	delete[] sText;
#endif
}

//Phase this out for now - Hydromorph
//void* InitUnhandledExceptionFilter() {
//
//	LPTOP_LEVEL_EXCEPTION_FILTER Handler = [] (LPEXCEPTION_POINTERS pExceptionInfo) -> LONG 
//	{
//		LogReport("==== DumpUnhandledException ==============================\r\n");
//
//		auto v6 = pExceptionInfo->ExceptionRecord;
//		LogReport("Fault Address : %08X\r\n", v6->ExceptionAddress);
//		LogReport("Exception code: %08X\r\n", v6->ExceptionCode);
//
//		auto v8 = pExceptionInfo->ContextRecord;
//		LogReport("Registers:\r\n");
//		LogReport(
//			"EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n",
//			v8->Eax, v8->Ebx, v8->Ecx,
//			v8->Edx, v8->Esi, v8->Edi);
//		LogReport("CS:EIP:%04X:%08X\r\n", v8->SegCs, v8->Eip);
//		LogReport("SS:ESP:%04X:%08X  EBP:%08X\r\n", v8->SegSs, v8->Esp, v8->Ebp);
//		LogReport("DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", v8->SegDs, v8->SegEs, v8->SegFs, v8->SegGs);
//		LogReport("Flags:%08X\r\n", v8->EFlags);
//
//		LogReport("\r\n");
//
//		return EXCEPTION_EXECUTE_HANDLER;
//	};
//
//	return SetUnhandledExceptionFilter(Handler);
//}

//Thanks Benny
void* InitVectoredExceptionHandler() {
	typedef const struct
	{
		unsigned int magic;
		void* object;
		_ThrowInfo* info;
	} CxxThrowExceptionObject;

	PVECTORED_EXCEPTION_HANDLER Handler = [](EXCEPTION_POINTERS* pExceptionInfo) -> long
	{
		if (pExceptionInfo->ExceptionRecord->ExceptionCode == 0xE06D7363) // C++ Exceptions
		{
			CxxThrowExceptionObject* exception_object = reinterpret_cast<CxxThrowExceptionObject*>(pExceptionInfo->ExceptionRecord->ExceptionInformation);

			if (exception_object->magic == 0x19930520 && exception_object->info->pCatchableTypeArray->nCatchableTypes > 0)
			{
				Log("CMSException: %s", exception_object->info->pCatchableTypeArray->arrayOfCatchableTypes[0]->pType->name);
				
				//TODO: Figure out MS2 Exception Object
				//ZException* exc = reinterpret_cast<ZException*>(exception_object->object);
				//printf("CMSException: %s - %08X\n", exception_object->info->pCatchableTypeArray->arrayOfCatchableTypes[0]->pType->name, exc->m_hr);
			}
		}
		else if (pExceptionInfo->ExceptionRecord->ExceptionCode != STATUS_PRIVILEGED_INSTRUCTION && pExceptionInfo->ExceptionRecord->ExceptionCode != DBG_PRINTEXCEPTION_C)
		{
			if (pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_HEAP_CORRUPTION || pExceptionInfo->ExceptionRecord->ExceptionCode == STATUS_ACCESS_VIOLATION)
			{
				if (pExceptionInfo->ExceptionRecord->ExceptionAddress != 0)
				{
					Log("RegException: %08X (%08X)", pExceptionInfo->ExceptionRecord->ExceptionCode, pExceptionInfo->ExceptionRecord->ExceptionAddress);

					//FILE* ff = fopen("C:\\exc.txt", "a+");

					//for (int i = 0; i < 0x400; i += 4)
					//	fprintf(ff, "Esp+%02X = %08X\n", i, *(DWORD*)(pExceptionInfo->ContextRecord->Esp + i));

					//fclose(ff);
				}
			}
			else
			{
				Log("RegException: %08X (%08X)", pExceptionInfo->ExceptionRecord->ExceptionCode, pExceptionInfo->ExceptionRecord->ExceptionAddress);
			}
		}

		return EXCEPTION_CONTINUE_SEARCH;
	};

	return AddVectoredExceptionHandler(1, Handler);
}

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
