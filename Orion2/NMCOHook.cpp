/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Dan
* @author Rajan
* @author Eric
*
*/
#include "Orion.h"
#include "NMCOHook.h"
#include "NMCO\NMGeneral.h"
#include "NMCO\NMFunctionObject.h"
#include "NMCO\NMSerializable.h"

typedef BOOL(__cdecl* pNMCO_CallNMFunction)(int uFuncCode, BYTE* pCallingData, char pUnk, BYTE**ppReturnData, UINT32& uReturnDataLen);
typedef BOOL(__cdecl* pNMCO_CallNMFunc)(int uFuncCode, BYTE* pCallingData, BYTE**ppReturnData, UINT32& uReturnDataLen);
typedef BOOL(__cdecl* pNMCO_CallNMFunc2)(int uFuncCode, BYTE* pCallingData, char pUnk, BYTE**ppReturnData, UINT32& uReturnDataLen);

pNMCO_CallNMFunction NMCO_CallNMFunction;
pNMCO_CallNMFunc NMCO_CallNMFunc;
pNMCO_CallNMFunc2 NMCO_CallNMFunc2;

BOOL InitNMFunctionHook(HMODULE hModule);
BOOL InitNMFuncHook(HMODULE hModule);
BOOL InitNMFunc2Hook(HMODULE hModule);

BOOL NMCO_CallNMFunction_Hook(int uFuncCode, BYTE* pCallingData, char pUnk, BYTE**ppReturnData, UINT32& uReturnDataLen);
BOOL NMCO_CallNMFunc_Hook(int uFuncCode, BYTE* pCallingData, BYTE**ppReturnData, UINT32& uReturnDataLen);
BOOL NMCO_CallNMFunc2_Hook(int uFuncCode, BYTE* pCallingData, char pUnk, BYTE**ppReturnData, UINT32& uReturnDataLen);

char* username = new char[PASSPORT_SIZE];

BOOL NMCOHook_Init() {
	HMODULE nmcoModule = LoadLibrary("nmcogame");

	return InitNMFuncHook(nmcoModule);
}

BOOL InitNMFunctionHook(HMODULE hModule) {
	if (!hModule)
		return FALSE;
	DWORD nmFuncAddr = (DWORD)GetProcAddress(hModule, "NMCO_CallNMFunction");
	if (!nmFuncAddr)
		return FALSE;
	NMCO_CallNMFunction = (pNMCO_CallNMFunction)nmFuncAddr;
	if (!SetHook(true, (PVOID*)&NMCO_CallNMFunction, (PVOID)NMCO_CallNMFunction_Hook))
		return FALSE;
	return TRUE;
}

BOOL InitNMFuncHook(HMODULE hModule) {
	if (!hModule)
		return FALSE;
	DWORD nmFuncAddr = (DWORD)GetProcAddress(hModule, "NMCO_CallNMFunc");
	if (!nmFuncAddr)
		return FALSE;
	NMCO_CallNMFunc = (pNMCO_CallNMFunc)nmFuncAddr;
	if (!SetHook(true, (PVOID*)&NMCO_CallNMFunc, (PVOID)NMCO_CallNMFunc_Hook))
		return FALSE;
	return TRUE;
}

BOOL InitNMFunc2Hook(HMODULE hModule) {
	if (!hModule)
		return FALSE;
	DWORD nmFuncAddr = (DWORD)GetProcAddress(hModule, "NMCO_CallNMFunc2");
	if (!nmFuncAddr)
		return FALSE;
	NMCO_CallNMFunc2 = (pNMCO_CallNMFunc2)nmFuncAddr;
	if (!SetHook(true, (PVOID*)&NMCO_CallNMFunc2, (PVOID)NMCO_CallNMFunc2_Hook))
		return FALSE;
}

BOOL NMCO_CallNMFunction_Hook(int uFuncCode, BYTE* pCallingData, char pUnk, BYTE**ppReturnData, UINT32& uReturnDataLen) {
	//CWvsApp::InitializeAuth
	int nEsi = 0;
	_asm mov nEsi, esi
	if (uFuncCode == kNMFuncCode_SetLocale || uFuncCode == kNMFuncCode_Initialize)
	{
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMSetLocaleFunc* retFunc = new CNMSetLocaleFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LoginAuth)
	{
		char* nm_username = reinterpret_cast<char*>(nEsi + 0x001030);
		char* nm_password = reinterpret_cast<char*>(nEsi + 0x001130);
		memcpy(username, nm_username, PASSPORT_SIZE); // nm_username +2 if \r\n
													  //CNMSimpleStream	ssStream;
													  //ssStream.SetBuffer(pCallingData);

													  //CNMLoginAuthFunc pFunc;
													  //pFunc.SetCalling();
													  //pFunc.DeSerialize(ssStream);
													  //MessageBoxFormat("szNexonID=%s\nszPassport=%s\nszPassword=%s", pFunc.szNexonID, pFunc.szPassport, pFunc.szPassword);
													  //memcpy(username, pFunc.szNexonID, PASSPORT_SIZE);

													  // Return to the client that login was successful.. NOT
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMLoginAuthFunc* retFunc = new CNMLoginAuthFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->nErrorCode = kLoginAuth_OK;
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_GetNexonPassport)
	{
		CNMSimpleStream* ssStream = new CNMSimpleStream(); // Memleaked actually. 

		CNMGetNexonPassportFunc* pFunc = new CNMGetNexonPassportFunc(); // Memleaked actually. 
		pFunc->bSuccess = true;

		strcpy(pFunc->szNexonPassport, username);

		pFunc->SetReturn();

		if (pFunc->Serialize(*ssStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = ssStream->GetBufferPtr();
		uReturnDataLen = ssStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LogoutAuth)
	{
		return TRUE;
	}


	NotifyMessage("NMCO_CallNMFunction: Found unhandled uFuncCode.", Orion::NotifyType::None);

	return NMCO_CallNMFunction(uFuncCode, pCallingData, pUnk, ppReturnData, uReturnDataLen);
}

BOOL NMCO_CallNMFunc_Hook(int uFuncCode, BYTE* pCallingData, BYTE**ppReturnData, UINT32& uReturnDataLen) {
	//CWvsApp::InitializeAuth
	if (uFuncCode == kNMFuncCode_SetLocale || uFuncCode == kNMFuncCode_Initialize)
	{
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMSetLocaleFunc* retFunc = new CNMSetLocaleFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LoginAuth)
	{
		CNMSimpleStream	ssStream;
		ssStream.SetBuffer(pCallingData);

		CNMLoginAuthFunc pFunc;
		pFunc.SetCalling();
		pFunc.DeSerialize(ssStream);
		//NotifyDbgMessage("szNexonID=%s\nszPassport=%s\nszPassword=%s", pFunc.szNexonID, pFunc.szPassport, pFunc.szPassword);
		memcpy(username, pFunc.szNexonID, PASSPORT_SIZE);
		//printf("Username: %s\r\n", username);

		// Return to the client that login was successful.. NOT
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMLoginAuthFunc* retFunc = new CNMLoginAuthFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->nErrorCode = kLoginAuth_OK;
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_GetNexonPassport)
	{
		CNMSimpleStream* ssStream = new CNMSimpleStream(); // Memleaked actually. 

		CNMGetNexonPassportFunc* pFunc = new CNMGetNexonPassportFunc(); // Memleaked actually. 
		pFunc->bSuccess = true;

		strcpy(pFunc->szNexonPassport, username);

		pFunc->SetReturn();

		if (pFunc->Serialize(*ssStream) == false)
			NotifyMessage("Could not Serialize?!", Orion::NotifyType::None);

		*ppReturnData = ssStream->GetBufferPtr();
		uReturnDataLen = ssStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LogoutAuth)
	{
		return TRUE;
	}


	NotifyMessage("NMCO_CallNMFunc: Found unhandled uFuncCode.", Orion::NotifyType::None);

	return NMCO_CallNMFunc(uFuncCode, pCallingData, ppReturnData, uReturnDataLen);
}

BOOL NMCO_CallNMFunc2_Hook(int uFuncCode, BYTE* pCallingData, char pUnk, BYTE**ppReturnData, UINT32& uReturnDataLen) {
	//CWvsApp::InitializeAuth
	if (uFuncCode == kNMFuncCode_SetLocale || uFuncCode == kNMFuncCode_Initialize)
	{
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMSetLocaleFunc* retFunc = new CNMSetLocaleFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LoginAuth)
	{
		CNMSimpleStream	ssStream;
		ssStream.SetBuffer(pCallingData);

		CNMLoginAuthFunc pFunc;
		pFunc.SetCalling();
		pFunc.DeSerialize(ssStream);
		//NotifyDbgMessage("szNexonID=%s\nszPassport=%s\nszPassword=%s", pFunc.szNexonID, pFunc.szPassport, pFunc.szPassword);
		memcpy(username, pFunc.szNexonID, PASSPORT_SIZE);
		//printf("Username: %s\r\n", username);

		// Return to the client that login was successful.. NOT
		CNMSimpleStream* returnStream = new CNMSimpleStream(); // Memleaked actually. 
		CNMLoginAuthFunc* retFunc = new CNMLoginAuthFunc(); // Memleaked actually. 
		retFunc->SetReturn();
		retFunc->nErrorCode = kLoginAuth_OK;
		retFunc->bSuccess = true;

		if (retFunc->Serialize(*returnStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = returnStream->GetBufferPtr();
		uReturnDataLen = returnStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_GetNexonPassport)
	{
		CNMSimpleStream* ssStream = new CNMSimpleStream(); // Memleaked actually. 

		CNMGetNexonPassportFunc* pFunc = new CNMGetNexonPassportFunc(); // Memleaked actually. 
		pFunc->bSuccess = true;

		strcpy(pFunc->szNexonPassport, username);

		pFunc->SetReturn();

		if (pFunc->Serialize(*ssStream) == false)
			MessageBoxA(NULL, "Could not Serialize?!", 0, 0);

		*ppReturnData = ssStream->GetBufferPtr();
		uReturnDataLen = ssStream->GetBufferSize();

		return TRUE;
	}
	else if (uFuncCode == kNMFuncCode_LogoutAuth)
	{
		return TRUE;
	}


	NotifyMessage("NMCO_CallNMFunc2: Found unhandled uFuncCode.", Orion::NotifyType::None);

	return NMCO_CallNMFunc2(uFuncCode, pCallingData, pUnk, ppReturnData, uReturnDataLen);
}
