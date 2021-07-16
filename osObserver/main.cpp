#include <process.h>
#include <algorithm>
#include <iostream>
#include "CPUDetect.h"
#include "util.h"

#define EXPORT __declspec(dllexport)
CPUDetect::CPUData g_cpuData = {};

class EXPORT ThreadInfo
{
public:
	HANDLE	hThread;
	DWORD	dwID;
	int		iPriority;
	DWORD_PTR	dwAffinityMask;
	DWORD		dwIdealProcessor;
	DWORD		dwCurrentProcessor;
	bool	bPowerThrotlling;
	bool	bUpdated;

public: 
	ThreadInfo(HANDLE cpuThread) {
		hThread = cpuThread;
		dwID = GetThreadId(hThread);
		dwAffinityMask = 0; //g_ProcessAffinityMask;
		dwCurrentProcessor = GetCurrentProcessorNumber();
		dwIdealProcessor = SetThreadIdealProcessor(hThread, MAXIMUM_PROCESSORS);
		iPriority = GetThreadPriority(hThread);
		bPowerThrotlling = false;
		bUpdated = false;
	}

	LPCWSTR GetThreadCurrentProcessorStates()
	{
		static WCHAR wcResult[128] = { 0 };
		char s[65] = {};

		DWORD pn = dwCurrentProcessor;
		for (int i = 0; i < g_cpuData.coreInfo.logicalProcessorCount; i++)
		{
			s[i] = (i == pn) ? '*' : '0';
		}
		blog_info("Thread Current Processor: %2u  [%s] ", pn, s);
		return wcResult;
	}

	void UpdateThreadInformation() {
		dwCurrentProcessor = GetCurrentProcessorNumber();
	}
};


EXPORT void setPowerThrotlling(ThreadInfo* threadInfo) {
	THREAD_POWER_THROTTLING_STATE PowerThrottling;
	RtlZeroMemory(&PowerThrottling, sizeof(PowerThrottling));
	PowerThrottling.Version = THREAD_POWER_THROTTLING_CURRENT_VERSION;
	PowerThrottling.ControlMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;
	PowerThrottling.StateMask = THREAD_POWER_THROTTLING_EXECUTION_SPEED;

	bool g_Error = !SetThreadInformation(threadInfo->hThread,
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
EXPORT void setAffinity(ThreadInfo* threadInfo, DWORD_PTR affinityMask) {
	bool g_Error = !SetThreadAffinityMask(threadInfo->hThread, affinityMask);
	if (g_Error)
	{
		blog_info("ERROR! Set thread affinity mask (0x%08X) failed!", affinityMask);
	}
	else
	{
		threadInfo->dwAffinityMask = affinityMask;
	}
}


int main(int argc, char* argv[]) {
	std::string mode = argv[1];  /*small; big ; all*/
	CPUDetect::InitCPUInfo(g_cpuData);

	HANDLE mainThread = GetCurrentThread();
	ThreadInfo	g_pInspectedThreadInfo(mainThread);

	if (mode == "small") {
		setAffinity(&g_pInspectedThreadInfo, g_cpuData.coreInfo.smallLogicalProcessorMask);  //logicalProcessorMask;smallLogicalProcessorMask
	}
	else if (mode == "big") {
		setAffinity(&g_pInspectedThreadInfo, g_cpuData.coreInfo.bigLogicalProcessorMask);  //logicalProcessorMask;smallLogicalProcessorMask
	}
	else if(mode == "all"){
		setAffinity(&g_pInspectedThreadInfo, g_cpuData.coreInfo.logicalProcessorMask);  //logicalProcessorMask;smallLogicalProcessorMask
	}
	else if (mode == "powerthrottle") {
		setPowerThrotlling(&g_pInspectedThreadInfo);
	}

	int count = 0;
	while (true) {
		Sleep(1000);
		g_pInspectedThreadInfo.UpdateThreadInformation();
		g_pInspectedThreadInfo.GetThreadCurrentProcessorStates();
		count++;
		if (count == 100) break;
	}

	return 0;
}

