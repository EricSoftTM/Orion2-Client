/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Eric
*
*/
#ifndef ORIONHACKS_H
#define ORIONHACKS_H

#include "Orion.h"

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

/* Null byte operation code */
#define EMPTY		0x00
/* Jump Equal operation code */
#define JE			0x74
/* Jump Short operation code */
#define JMP_SHORT	0xEB
/* Jump Long operation code */
#define JMP_LONG	0xE9
/* No-Op operation code */
#define NOP			0x90
/* Move operation code */
#define MOV			0xB8//WriteMOV? TODO

/* Enable all client address memory edits */
bool InitializeOrion2();
/* Enables Multi-Client */
bool InitializeMultiClient();

/* Memory writers */
void WriteAoB(DWORD, const char*);
void WriteByte(DWORD, unsigned char);
void WriteDouble(DWORD, double);
void WriteInt(DWORD, int);
void WriteJMP(DWORD dwAddress, DWORD dwLocation = 0, DWORD dwOffset = 0, int nNOP = 0);
void WriteNOP(DWORD dwAddress, DWORD dwOffset = 0, int nCount = 1);
void WriteNull(DWORD dwAddress, DWORD dwOffset = 0, int nCount = 1);
void WriteString(DWORD, const char*);

/* Additional helper functions */
DWORD FindAoB(const char*, DWORD, DWORD, int);
unsigned int ReadAoB(const char*, unsigned char*, bool*);

#endif