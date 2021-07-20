#include <process.h>
#include <algorithm>
#include <iostream>
#include "CPUDetect.h"
#include "util.h"

int main(int argc, char* argv[]) {
	std::string mode = argv[1];  /*small; big ; all*/
	CPUDetect cpuDetect;
	cpuDetect.InitCPUInfo(cpuDetect.cpuData);

	HANDLE mainThread = GetCurrentThread();
	ThreadInfo	g_pInspectedThreadInfo(mainThread);

	if (mode == "small") {
		setAffinity(&g_pInspectedThreadInfo, cpuDetect.cpuData.coreInfo.smallLogicalProcessorMask);  //logicalProcessorMask;smallLogicalProcessorMask
	}
	else if (mode == "big") {
		setAffinity(&g_pInspectedThreadInfo, cpuDetect.cpuData.coreInfo.bigLogicalProcessorMask);  //logicalProcessorMask;smallLogicalProcessorMask
	}
	else if(mode == "all"){
		setAffinity(&g_pInspectedThreadInfo, cpuDetect.cpuData.coreInfo.logicalProcessorMask);  //logicalProcessorMask;smallLogicalProcessorMask
	}
	else if (mode == "powerthrottle") {
		setPowerThrotlling(&g_pInspectedThreadInfo);
	}

	int count = 0;
	while (true) {
		Sleep(1000);
		g_pInspectedThreadInfo.UpdateThreadInformation();
		g_pInspectedThreadInfo.GetThreadCurrentProcessorStates(cpuDetect.cpuData);
		count++;
		if (count == 100) break;
	}

	return 0;
}

