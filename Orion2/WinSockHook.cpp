/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Benny
* @author Dan
* @author Eric
*
*/
#include "WinSockHook.h"
#include "NMCOHook.h"

/* WSPConnect */
static LPWSPCONNECT _WSPConnect = NULL;
/* WSPGetPeerName */
static LPWSPGETPEERNAME _WSPGetPeerName = NULL;
/* WSPStartup */
static LPWSPSTARTUP _WSPStartup = NULL;

/* The original socket host address */
DWORD dwHostAddress = 0;
/* The re-routed socket host address */
DWORD dwRouteAddress = 0;

/* Retrieve the IP address to connect to from configuration, otherwise default to local */
const char* GetClientIP() {
	const char* sDefaultIP = "127.0.0.1";

	char sAddr[16];
	if (GetPrivateProfileStringA("Settings", "ClientIP", sDefaultIP, sAddr, sizeof(sAddr), ".\\Orion.ini")) {
		return sAddr;
	}
	return sDefaultIP;
}

/* Hooks the Winsock Service Provider's Connect function to redirect the host to a new socket */
int WINAPI WSPConnect_Hook(SOCKET s, sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno) {
	/* Retrieve a string buffer of the current socket address (IP) */
	char pBuff[50];
	DWORD dwStringLength = 50;
	WSAAddressToStringA(name, namelen, NULL, pBuff, &dwStringLength);

	sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(name);
	unsigned short pPort = htons(addr->sin_port);

	if (strstr(pBuff, NULL_IP)) {
		if (!pPort) {
			Log("Unable to resolve IP and Port information. Socket redirection failed.");
			return FALSE;
		} else Log("Warning: IP address was null! It has been defaulted to %s because a valid port was provided.", GetClientIP());
	}

	Log("[WSPConnect_Hook] Address: %s => Port: %d", pBuff, pPort);

	if (strstr(pBuff, NEXON_IP_NA) || strstr(pBuff, NEXON_IP_SA) || strstr(pBuff, NEXON_IP_EU) || strstr(pBuff, NULL_IP)) {
		/* Initialize the re-reoute socket address to redirect to */
		hostent* he = gethostbyname(GetClientIP());
		if (!he) {
			NotifyMessage("The server is unable to connect or is currently offline.", Orion::NotifyType::Error);
			ExitProcess(0);
			return FALSE;
		}

		VM_START
		memcpy(&dwRouteAddress, he->h_addr_list[0], he->h_length);
		Log("[WSPConnect_Hook] Patching to new address: %s", GetClientIP());

		/* Copy the original host address and back it up */
		memcpy(&dwHostAddress, &addr->sin_addr, sizeof(DWORD));
		/* Update the host address to the route address */
		memcpy(&addr->sin_addr, &dwRouteAddress, sizeof(DWORD));
		VM_END
	}
	else
	{
		Log("[WSPConnect_Hook] Connecting to socket address: %s", pBuff);
	}

	return _WSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}

/* Hooks the Winsock Service Provider's GetPeerName function to pretend to be connected to the host */
int WINAPI WSPGetPeerName_Hook(SOCKET s, sockaddr* name, LPINT namelen, LPINT lpErrno) {
	int nResult = _WSPGetPeerName(s, name, namelen, lpErrno);

	VM_START
	if (nResult == 0) {
		sockaddr_in* addr = reinterpret_cast<sockaddr_in*>(name);

		/* Check if the returned address is the routed address */
		if (addr->sin_addr.S_un.S_addr == dwRouteAddress) {
			/* Return the socket address back to the host address */
			memcpy(&addr->sin_addr, &dwHostAddress, sizeof(DWORD));
		}
	}
	VM_END

	return nResult;
}

/* Hooks the Winsock Service Provider's Startup function to initiate the SPI and spoof the socket */
bool Hook_WSPStartup(bool bEnable) {
	/* Initialize the WSPStartup module and jump to hook if successful */
	if (!_WSPStartup) {
		HMODULE hModule = LoadLibraryA("MSWSOCK");

		VM_START
		if (hModule) {
			_WSPStartup = reinterpret_cast<LPWSPSTARTUP>(GetProcAddress(hModule, "WSPStartup"));

			if (_WSPStartup) {
				goto Hook;
			}
		}
		VM_END

		return false;
	}

Hook:
	LPWSPSTARTUP WSPStartup_Hook = [](WORD wVersionRequested, LPWSPDATA lpWSPData, LPWSAPROTOCOL_INFOW lpProtocolInfo, WSPUPCALLTABLE UpcallTable, LPWSPPROC_TABLE lpProcTable) -> int {
		int nResult = _WSPStartup(wVersionRequested, lpWSPData, lpProtocolInfo, UpcallTable, lpProcTable);

		if (nResult == 0) {
			/* Redirect WSPConnect to our hook */
			_WSPConnect = lpProcTable->lpWSPConnect;
			lpProcTable->lpWSPConnect = reinterpret_cast<LPWSPCONNECT>(WSPConnect_Hook);

			/* Redirect WSPGetPeerName to our hook */
			_WSPGetPeerName = lpProcTable->lpWSPGetPeerName;
			lpProcTable->lpWSPGetPeerName = WSPGetPeerName_Hook;
		}

		return nResult;
	};

	/* Enable the WSPStartup hook */
	return SetHook(bEnable, reinterpret_cast<void**>(&_WSPStartup), WSPStartup_Hook);
}