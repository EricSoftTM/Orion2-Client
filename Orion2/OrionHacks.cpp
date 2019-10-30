/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Eric
*
*/
#include "OrionHacks.h"
#include "Themida/ThemidaSDK.h"

/* Initializes all memory alterations used for Orion2 */
bool InitializeOrion2() {
	VM_START

	DWORD dwSwearFilter = FindAoB("83 CF FF 8D 4D EC 89 7D FC FF 15 ?? ?? ?? 01 68 ?? ?? ?? 01 8D 4D EC FF 15 ?? ?? ?? 01 8D 4E 5C", 0, 0, 0) + 0x3D;//Confirmed v24~v55 (v9 is +0x33)
	DWORD dwNXLBypass1 = FindAoB("E8 ?? ?? ?? FF ?? ?? ?? ?? ?? ?? ?? 00 ?? ?? ?? ?? ?? ?? ?? 8B ?? 8B 42 ?? FF D0 ?? ?? ?? ?? ?? ?? ??", 0, 0, 0);//Confirmed v1~v55
	DWORD dwNXLBypass2 = FindAoB("83 C4 04 85 C0 74 08 33 C0 5F 5E 8B E5 5D C3 E8 ?? ?? ?? FF 85 C0 ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ??", 0, 0, 0) + 0xF;//Confirmed v1~v55
	DWORD dwDisableNXL = FindAoB("B8 01 00 00 00 C3 CC CC CC CC CC CC CC CC CC CC 55 8B EC 6A FF 68 ?? ?? ?? 01 64 A1 00 00 00 00", 0, 0, 2);//v1 & current: 2 skips, v9~v70: 1 skip
	// If dwDisableNXL does NOT enable LoginUI for user/pass, change the skips around from 1 and 2.
	// On the original GMST2 clients (as well as KMST2), the client used 2 skips. 
	// Then, from v9~v70 (much higher, not sure where it ended), the client used 1 skip.
	// As of current GMS2 , the client is once again using 2 skips.

	bool bInit = true;
	if (DISABLE_NXL) {
		bInit &= (dwDisableNXL > PE_START);
		if (bInit) {
			WriteAoB(dwDisableNXL, "B8 00 00 00 00");
		}
	} else {
		bInit &= (dwNXLBypass1 > PE_START) && (dwNXLBypass2 > PE_START);
		if (bInit) {
			/* Bypass Nexon Launcher Server Checks */
			WriteAoB(dwNXLBypass1, "B8 00 00 00 00");

			/* Bypass Nexon Launcher IP Checks */
			WriteAoB(dwNXLBypass2, "B8 00 00 00 00");
		}
	}

	if (bInit) {
		if (CHAT_SPAM) {
			// TODO: Figure out how to bypass chat spam detection.
		}

		if (CHAT_LENGTH > 50) {
			// TODO: Figure out how to modify the chat length cap (default max: 50).
		}

		if (SWEAR_FILTER) {
			/* Bypasses the "banWord" checks to allow cursing. */
			WriteAoB(dwSwearFilter, "90 90 90 90 90");
		}
	}
	
	VM_END
	return bInit;
}

/**
* Searches through memory to find a match to the given AoB
*
* @param sAoB The AoB string to search for in memory
* @param dwStartAddress The address to start the search at (defaults to PE start)
* @param dwEndAddress The ending address of where to not search any further from
* @param nSkip The amount of successful matches to skip before returning (default 0)
*
*/
DWORD FindAoB(const char* sAoB, DWORD dwStartAddress, DWORD dwEndAddress, int nSkip) {
	BYTE pBuff[MAX_BUFFER] = { 0x00 };
	bool aMask[MAX_BUFFER] = { 0x00 };
	unsigned int uSize = ReadAoB(sAoB, pBuff, aMask);
	unsigned int i, j;
	int nSkipped = 0;

	if (uSize > 0) {
		dwStartAddress = dwStartAddress ? dwStartAddress : 0x00400000;
		dwEndAddress = dwEndAddress ? dwEndAddress : 0x07FFFFFF;

		__try {
			for (i = dwStartAddress; i < dwEndAddress; i++) {
				for (j = 0; j < uSize; j++) {
					if (aMask[j]) {
						continue;
					}
					if (pBuff[j] != *(BYTE *)(i + j)) {
						break;
					}
				}
				if (j == uSize) {
					nSkipped++;
					if (nSkipped >= nSkip) {
						return i;
					}
				}
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			return 0;
		}
	}

	return 0;
}

/**
* Reads an AoB string and outputs the correct bytes/instructions to a new buffer
*
* @param sAoB The raw AoB string to read
* @param pBuff The newly corrected buffer to search for
* @param pMask A mask pointer determining if the index was ??
*
*/
unsigned int ReadAoB(const char* sAoB, unsigned char* pBuff, bool* pMask) {
	unsigned char bOp;
	int i = 0, j = 0;

	while ((bOp = (BYTE)sAoB[j]), bOp) {
		unsigned char bFlag = (i % 2 == 0) ? 4 : 0;

		if (bOp >= '0' && bOp <= '9')
			pBuff[i++ / 2] += (bOp - '0') << bFlag;
		else if (bOp >= 'A' && bOp <= 'F')
			pBuff[i++ / 2] += (bOp - 'A' + 0xA) << bFlag;
		else if (bOp >= 'a' && bOp <= 'f')
			pBuff[i++ / 2] += (bOp - 'a' + 0xA) << bFlag;
		else if (bOp == '?') {
			pBuff[i / 2] = 0xFF;
			pMask[i / 2] = true;
			i++;
		}
		j++;
	}

	return (i % 2 == 0) ? (i / 2) : -1;
}

/**
* Writes an AoB (Array of Bytes) directly to memory at specified address.
*
* Simply put, this method will convert [00 00 00 00] to [\x00\x00\x00\x00],
* and then copy the bytes into memory at the address provided.
*
*/
void WriteAoB(DWORD dwAddress, const char* sBytes) {
	// Calculate the length of the AoB
	const unsigned int uLen = strlen(sBytes);
	// Initialize the new, real length of memory to copy
	unsigned int uBufLen = 1;
	// Construct a new buffer for the fixed AoB
	BYTE pBuff[MAX_BUFFER] = { 0x00 };

	// Not enough bytes, invalid AoB
	if (uLen < 2 || uLen > MAX_BUFFER) return;

	int i = 0;
	BYTE bInstruction;
	// Continue looping to further replace all AoB ASCII with appropriate HEX
	while ((bInstruction = (BYTE)*sBytes++), bInstruction) {
		BYTE bFlag = (i % 2 == 0) ? 4 : 0;//HIGH/LOW

		if (bInstruction >= '0' && bInstruction <= '9')
			pBuff[i++ / 2] += (bInstruction - '0') << bFlag;
		else if (bInstruction >= 'A' && bInstruction <= 'F')
			pBuff[i++ / 2] += (bInstruction - 'A' + 0xA) << bFlag;
		else if (bInstruction >= 'a' && bInstruction <= 'f')
			pBuff[i++ / 2] += (bInstruction - 'a' + 0xA) << bFlag;

		if (bInstruction == ' ')
			++uBufLen;
	}

	DWORD dwOldProtect;
	DWORD dwOldAddr;
	DWORD dwOldSize;
	__try {
		/* Since the location in memory is Read Only, obtain Write Access and then modify pointer */
		if (VirtualProtect((DWORD *) dwAddress, uBufLen, PAGE_EXECUTE_READWRITE, &dwOldProtect)) {
			dwOldAddr = (DWORD)dwAddress;
			dwOldSize = uBufLen;

			/* Copy the new buffer to the address given, writing uBufLen bytes */
			memcpy(reinterpret_cast<void*>(dwAddress), pBuff, uBufLen);

			/* Restore the Read Only protection at the given location */
			DWORD old;
			VirtualProtect((DWORD *)dwOldAddr, dwOldSize, dwOldProtect, &old);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
}

/* Writes an unsigned byte to memory at the provided address */
void WriteByte(DWORD dwAddress, unsigned char bData) {
	*(unsigned char *)dwAddress = bData;
}

/* Writes an integer to memory at the provided address */
void WriteInt(DWORD dwAddress, int nData) {
	*(int *)dwAddress = nData;
}

/* Quick and simple way to write multiple empty bytes at a specific address and offset */
void WriteNull(DWORD dwAddress, DWORD dwOffset, int nCount) {
	unsigned char* pBuff = (unsigned char *)(dwAddress + dwOffset);

	for (int i = 0; i < nCount; i++)
		WriteByte((DWORD)(pBuff + i), EMPTY);
}

/* Quick and simple way to JMP to a specific address and pad with NOPs if needed */
void WriteJMP(DWORD dwAddress, DWORD dwLocation, DWORD dwOffset, int nNOP) {
	unsigned char* pBuff = (unsigned char *)(dwAddress + dwOffset);

	if (!dwLocation) {
		WriteByte((DWORD)pBuff, JMP_SHORT);
	}
	else {
		WriteByte((DWORD)pBuff, JMP_LONG);
		WriteInt((DWORD)(pBuff + 1), (dwLocation - (DWORD)pBuff) - 5);
	}

	int i = sizeof(unsigned char);
	if (dwLocation)
		i += sizeof(int);
	for (; i < nNOP; i++)
		WriteByte((DWORD)(pBuff + i), NOP);
}

/* Quick and simple way to NOP multiple bytes at a specific address and offset */
void WriteNOP(DWORD dwAddress, DWORD dwOffset, int nCount) {
	unsigned char* pBuff = (unsigned char *)(dwAddress + dwOffset);

	for (int i = 0; i < nCount; i++)
		WriteByte((DWORD)(pBuff + i), NOP);
}

/* Quick and simple way to write a string to memory at a specific address */
void WriteString(DWORD dwAddress, const char* sStr) {
	unsigned int uLen = strlen(sStr);

	unsigned int i = 0;
	for (; i < uLen; i++) {
		WriteByte(dwAddress + i, sStr[i]);
	}
	WriteByte(dwAddress + i, '\0');
}

/* Writes an 8-byte double to memory at the address specified */
void WriteDouble(DWORD dwAddress, double dValue) {
	union {
		long long liVal;
		double    dVal;
	} Result;

	Result.dVal = dValue;

	const unsigned int uSize = sizeof(double) * 2;//2 characters per byte
	char pBuff[uSize + 1];
	char pDest[uSize + sizeof(double)];

	_snprintf(pBuff, sizeof(pBuff), "%016llx", Result.liVal);
	pBuff[uSize] = '\0';

	unsigned int uLen = uSize + sizeof(double);

	pDest[--uLen] = '\0';
	for (unsigned int i = 0; i < uSize; i += 2) {
		pDest[--uLen] = pBuff[i + 1];
		pDest[--uLen] = pBuff[i];
		pDest[--uLen] = ' ';
	}

	WriteAoB(dwAddress, pDest);
}
