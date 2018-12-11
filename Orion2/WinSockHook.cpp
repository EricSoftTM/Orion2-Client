/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Benny
* @author Dan
* @author Eric
* @author Rajan
*
*/

#include "AddrQueue.h"
#include "WinSockHook.h"
#include "NMCOHook.h"

//TODO: When the time comes, Push migrations to AddrQueue

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

/* Hooks the Winsock Service Provider's Connect function to redirect the host to a new socket */
int WINAPI WSPConnect_Hook(SOCKET s, sockaddr* name, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS, LPINT lpErrno) {
	/* Retrieve a string buffer of the current socket address (IP) */
	char pBuff[50];
	DWORD dwStringLength = 50;
	WSAAddressToStringA(name, namelen, NULL, pBuff, &dwStringLength);

	auto bPatch = strstr(pBuff, NEXON_IP_NA) || strstr(pBuff, NEXON_IP_SA) || strstr(pBuff, NEXON_IP_EU);

	VM_START

		/* Initialize the re-reoute socket address to redirect to */
		if (bPatch) {
			auto pAddr = reinterpret_cast<sockaddr_in*>(name);
			
			DWORD dwAddr = pAddr->sin_addr.S_un.S_addr;
			SHORT nPort = pAddr->sin_port;

			/* Nothing in queue? Default to Login Server*/
			if (AddrQueue::Empty())
			{
				dwAddr = inet_addr(CLIENT_IP);
				//nPort = CLIENT_PORT;
			}
			else
			{
				auto head = AddrQueue::Front();

				dwAddr = head.dwAddr;
				nPort = head.nPort;

				AddrQueue::Pop();
			}
			
			/* Copy the original host address and back it up */
			dwRouteAddress = dwAddr;

			/* Update the host address to the route address */
			pAddr->sin_addr.S_un.S_addr = dwAddr;
			pAddr->sin_port = nPort;
		}

#if DEBUG_MODE
			printf("[WSPConnect_Hook] Connecting to socket: %s | Patched %d\n", pBuff, bPatch);
#endif
		
	VM_END
		return _WSPConnect(s, name, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS, lpErrno);
}

/* Hooks the Winsock Service Provider's GetPeerName function to pretend to be connected to the host */
int WINAPI WSPGetPeerName_Hook(SOCKET s, sockaddr* name, LPINT namelen, LPINT lpErrno) {
	int nResult = _WSPGetPeerName(s, name, namelen, lpErrno);

	if (nResult == 0) {
		auto pAddr = reinterpret_cast<sockaddr_in*>(name);

		/* Check if the returned address is the routed address */
		if (pAddr->sin_addr.S_un.S_addr == dwRouteAddress) {
			/* Return the socket address back to the host address */
			pAddr->sin_addr.S_un.S_addr = dwHostAddress;
		}
	}

	return nResult;
}

/* Hooks the Winsock Service Provider's Startup function to initiate the SPI and spoof the socket */
bool Hook_WSPStartup(bool bEnable) {
	/* Initialize the WSPStartup module and jump to hook if successful */
	if (!_WSPStartup) {
		VM_START
		HMODULE hModule = LoadLibraryA("MSWSOCK");

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
		VM_START
		int nResult = _WSPStartup(wVersionRequested, lpWSPData, lpProtocolInfo, UpcallTable, lpProcTable);

		if (nResult == 0) {
			/* Redirect WSPConnect to our hook */
			_WSPConnect = lpProcTable->lpWSPConnect;
			lpProcTable->lpWSPConnect = reinterpret_cast<LPWSPCONNECT>(WSPConnect_Hook);

			/* Redirect WSPGetPeerName to our hook */
			_WSPGetPeerName = lpProcTable->lpWSPGetPeerName;
			lpProcTable->lpWSPGetPeerName = WSPGetPeerName_Hook;
		}
		VM_END
		return nResult;
	};

	/* Enable the WSPStartup hook */
	return SetHook(bEnable, reinterpret_cast<void**>(&_WSPStartup), WSPStartup_Hook);
}