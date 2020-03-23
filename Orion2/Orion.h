/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Eric
*
*/
#pragma once

#ifndef ORION_H
#define ORION_H

#pragma comment(lib, "ws2_32.lib")

/* Global internal imports */
#include <Ws2tcpip.h> /* Begin Winsock */
#include <WS2spi.h>
#include <WinSock2.h>
#include <stdint.h>
#include <stdio.h> /* End Winsock */
#include <windows.h>
#include <winternl.h>
#include <tchar.h>
#include <strsafe.h>
#include <direct.h>
#include <iostream>

/* Global external imports */
#include "Detours.h"

/* Client constants */
#define CLIENT_NAME		"Orion2 - A New Beginning" /* The window title */
#define CLIENT_IP		"127.0.0.1" /* The server IP to reroute to */
#define CLIENT_PORT		20001 /* The server Port to connect to (Orion2: 20001, GMS2: 30000) */
#define CLIENT_LOCALE	"EN" /* The client's locale (used to be en-US, now it's EN) */
#define DEBUG_MODE		1 /* 0 = No Logging, 1 = General Logging, 2 = Enable everything */

/* Client hacks/customization constants */
#define CHAT_SPAM		FALSE /* Enable chat spam */
#define CHAT_LENGTH		127 /* Custom chat text length */
#define SWEAR_FILTER	FALSE /* Enable swear filter */
#define MULTI_CLIENT	TRUE /* Enable multi-client */
#define DISABLE_NXL		TRUE /* Universally disables all NXL functionality (enables LoginUI for user/pass) */

/* Other constants */
#define STRING_LOCALE	"en-US" /* The application's locale (korean: ko-KR, english: en-US, chinese: zh-CN) */
#define MAX_BUFFER		1024 /* Maximum buffer size used for various arrays */
#define PE_START		0x00401000 /* The standard PE start address */
#define PE_END			0x04FFFFFF /* The scan range of the PE */
#define NEXON_IP_NA		"23.98.21" /* Nexon's North America IP pattern to search for upon hook */
#define NEXON_IP_SA		"52.171.48" /* Nexon's South America IP pattern to search for upon hook */
#define NEXON_IP_EU		"13.65.17" /* Nexon's Europe IP pattern to search for upon hook */
#define MUTLI_MUTEX		"Global\\7D9D84AE-A653-4C89-A004-26E262ECE0C4" /* Nexon's mutex key for checking MS2 Multi-Client */
#define CLIENT_CLASS    "MapleStory2" /* The class name of the main MapleStory2 window */

class Orion {
	public:
		enum NotifyType {
			None		= 0,
			Information = MB_OK | MB_ICONINFORMATION,
			Error		= MB_OK | MB_ICONERROR
		};

};

/* Win32 detours */
BOOL SetHook(__in BOOL bInstall, __inout PVOID* ppvTarget, __in PVOID pvDetour);

/* Win32 hooks */
bool Hook_CreateWindowExA(bool);
bool Hook_GetCurrentDirectoryA(bool);
bool Hook_CreateMutexA(bool);
bool RedirectProcess();

void Log(const char*, ...);

//void* InitUnhandledExceptionFilter();
void* InitVectoredExceptionHandler();

/* MessageBox debugging */
void NotifyMessage(const char*);
void NotifyMessage(const char*, int);
void NotifyDbgMessage(const char*, ...);
void NotifyDbgMessageW(const wchar_t*, ...);

#endif