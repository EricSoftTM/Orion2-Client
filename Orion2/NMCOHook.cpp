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

char* g_UserName = new char[PASSPORT_SIZE];

typedef BOOL(__cdecl* pNMCO_CallNMFunc)(int uFuncCode, BYTE* pCallingData, BYTE**ppReturnData, UINT32& uReturnDataLen);

pNMCO_CallNMFunc NMCO_CallNMFunc;

BOOL NMCO_CallNMFunc_Hook(int uFuncCode, BYTE* pCallingData, BYTE**ppReturnData, UINT32& uReturnDataLen) {

	if (uFuncCode == kNMFuncCode_LogoutAuth)
	{
		return TRUE;
	}

	auto bPatch = true;

	CNMFunc* retFunc = nullptr;

	if (uFuncCode == kNMFuncCode_SetLocale || uFuncCode == kNMFuncCode_Initialize)
	{
		retFunc = new CNMSetLocaleFunc();
	}
	else if (uFuncCode == kNMFuncCode_LoginAuth)
	{
		CNMSimpleStream	ssStream;
		ssStream.SetBuffer(pCallingData);

		CNMLoginAuthFunc pFunc;
		pFunc.SetCalling();
		pFunc.DeSerialize(ssStream);

		memcpy(g_UserName, pFunc.szNexonID, PASSPORT_SIZE);
		//printf("Username: %s\r\n", g_UserName);

		auto curFunc = new CNMLoginAuthFunc();
		curFunc->nErrorCode = kLoginAuth_OK;

		retFunc = curFunc;
	}
	else if (uFuncCode == kNMFuncCode_GetNexonPassport)
	{
		auto curFunc = new CNMGetNexonPassportFunc();
		strcpy(curFunc->szNexonPassport, g_UserName);

		retFunc = curFunc;
	}
	else
	{
		bPatch = false;
	}

	if (bPatch)
	{
		retFunc->bSuccess = true;
		retFunc->SetReturn();

		CNMSimpleStream* retStream = new CNMSimpleStream(); // Mem Leak !!!

		if (retFunc->Serialize(*retStream) == false)
			NotifyMessage("Could not Serialize?!", Orion::NotifyType::None);

		*ppReturnData = retStream->GetBufferPtr();
		uReturnDataLen = retStream->GetBufferSize();

		return TRUE;
	}

	NotifyMessage("NMCO_CallNMFunc: Found unhandled uFuncCode.", Orion::NotifyType::None);
	return NMCO_CallNMFunc(uFuncCode, pCallingData, ppReturnData, uReturnDataLen);
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

BOOL NMCOHook_Init() {
	HMODULE nmcoModule = LoadLibrary("nmcogame");
	return InitNMFuncHook(nmcoModule);
}