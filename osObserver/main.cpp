#include <process.h>
#include <algorithm>
#include <iostream>
#include "CPUDetect.h"
#include "util.h"

DWORD_PTR	g_ProcessAffinityMask = 0;
bool		g_Error = false;
CPUDetect::CPUData g_cpuData = {};


struct ThreadInfo
{
	HANDLE	hThread;
	DWORD	dwID;
	int		iPriority;
	DWORD_PTR	dwAffinityMask;
	DWORD		dwIdealProcessor;
	DWORD		dwCurrentProcessor;
	bool	bPowerThrotlling;
	bool	bUpdated;
};

void InitThreadInformations(ThreadInfo threadInformations[], HANDLE threads[], size_t numThreads)
{
	HANDLE hThread;
	for (int iInstance = 0; iInstance < numThreads; ++iInstance)
	{
		hThread = threads[iInstance];
		threadInformations[iInstance].hThread = hThread;
		threadInformations[iInstance].dwID = GetThreadId(hThread);
		threadInformations[iInstance].dwAffinityMask = g_ProcessAffinityMask;
		threadInformations[iInstance].dwIdealProcessor = SetThreadIdealProcessor(hThread, MAXIMUM_PROCESSORS);
		threadInformations[iInstance].iPriority = GetThreadPriority(hThread);
		threadInformations[iInstance].bPowerThrotlling = false;
		threadInformations[iInstance].bUpdated = false;
	}
}

void setPowerThrotlling(ThreadInfo* threadInfo) {
	THREAD_POWER_THROTTLING_STATE PowerThrottling;
	RtlZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
	PowerThrottling.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
	PowerThrottling.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
	PowerThrottling.StateMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;

	g_Error = !SetThreadInformation(threadInfo->hThread,
	ThreadPowerThrottling,
	&PowerThrottling,
	sizeof(PowerThrottling));

	if (g_Error)
	{
		blog_info("ERROR! Set thread power throttling failed!");
	}
	else
	{
		threadInfo->bPowerThrotlling = true;
	}
}

void setAffinity(ThreadInfo* threadInfo, DWORD_PTR affinityMask) {
	g_Error = !SetThreadAffinityMask(threadInfo->hThread, affinityMask);
	if (g_Error)
	{
		blog_info("ERROR! Set thread affinity mask (0x%08X) failed!", affinityMask);
	}
	else
	{
		threadInfo->dwAffinityMask = affinityMask;
	}
}

LPCWSTR GetThreadCurrentProcessorStates(ThreadInfo* threadInfo)
{
	static WCHAR wcResult[128] = { 0 };
	char s[65] = {};

	DWORD pn = threadInfo->dwCurrentProcessor;
	for (int i = 0; i < g_cpuData.coreInfo.logicalProcessorCount; i++)
	{
		s[i] = (i == pn) ? '*' : '0';
	}
	blog_info("Thread Current Processor: %2u  [%s] ", pn, s);
	return wcResult;
}

int main() {
	CPUDetect::InitCPUInfo(g_cpuData);
	HANDLE mainThread = GetCurrentThread();
	ThreadInfo	g_pInspectedThreadInfo;
	InitThreadInformations(&g_pInspectedThreadInfo, &mainThread, 1);

	g_pInspectedThreadInfo.dwCurrentProcessor = GetCurrentProcessorNumber();
	setPowerThrotlling(&g_pInspectedThreadInfo);
	setAffinity(&g_pInspectedThreadInfo, g_cpuData.coreInfo.logicalProcessorMask);  //logicalProcessorMask;smallLogicalProcessorMask
	GetThreadCurrentProcessorStates(&g_pInspectedThreadInfo);
	return 0;
}

