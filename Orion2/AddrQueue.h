#pragma once

#include <Windows.h>

struct ConCtx
{
	unsigned int dwAddr;
	unsigned short nPort;
};

namespace AddrQueue
{
	bool Empty();

	ConCtx Front();

	void Push(int dwAddr, short nPort);
	void Pop();
}