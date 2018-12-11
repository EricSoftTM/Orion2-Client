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
#define CLIENT_NAME		"Orion2 - A New Beginning"
#define CLIENT_IP		"127.0.0.1"
#define ADMIN_CLIENT	FALSE
#define DEBUG_MODE		TRUE

/* Client hacks/customization constants */
#define CHAT_SPAM		FALSE /* Enable chat spam */
#define CHAT_LENGTH		127 /* Custom chat text length */
#define SWEAR_FILTER	TRUE /* Enable swear filter */
#define DROPPABLE_NX	FALSE /* Enable droppable NX */
#define ENABLE_IME		FALSE /* Enable Microsoft IME */
#define MULTI_CLIENT	FALSE /* Enable multi-client */
#define DISABLE_NXL		TRUE /* Universally disabled all NXL functionality (enables LoginUI for user/pass) */

/* Other constants */
#define STRING_LOCALE	"kor" /* The application's string locale by default is Korean(kor) */
#define NEXON_IP_NA		"23.98.21" /* Nexon's North America IP pattern to search for upon hook */
#define NEXON_IP_SA		"52.171.48" /* Nexon's South America IP pattern to search for upon hook */
#define NEXON_IP_EU		"13.65.17" /* Nexon's Europe IP pattern to search for upon hook */
#define MAX_BUFFER		1024 /* Maximum buffer size used for various arrays */
#define MUTLI_MUTEX		"Global\\7D9D84AE-A653-4C89-A004-26E262ECE0C4"
#define CLIENT_CLASS    "MapleStory2"

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

//void* InitUnhandledExceptionFilter();
void* InitVectoredExceptionHandler();

/* MessageBox debugging */
void NotifyMessage(const char*);
void NotifyMessage(const char*, int);
void NotifyDbgMessage(const char*, ...);
void NotifyDbgMessageW(const wchar_t*, ...);

#endif