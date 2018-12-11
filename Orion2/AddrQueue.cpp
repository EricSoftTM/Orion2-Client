#include <queue>
#include "AddrQueue.h"

namespace AddrQueue
{
	std::queue<ConCtx> g_ConCtxQueue;

	bool Empty()
	{
		return g_ConCtxQueue.empty();
	}

	ConCtx Front()
	{
		return g_ConCtxQueue.front();
	}

	void Push(int dwAddr, short nPort)
	{
		ConCtx ctx;
		ctx.dwAddr = dwAddr;
		ctx.nPort = nPort;

		g_ConCtxQueue.push(ctx);
	}
	void Pop()
	{
		g_ConCtxQueue.pop();
	}
}