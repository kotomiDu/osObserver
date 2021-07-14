////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
////////////////////////////////////////////////////////////////////////////////
#include "CPUDetect.h"
#include <intrin.h>
#include <assert.h>
#include "util.h"

namespace CPUDetect
{
	//--------------------------------------------------------------------------------------
	// Helper functions for querying information about the processors in the current
	// system.  ( Copied from the doc page for GetLogicalProcessorInformation() )
	//--------------------------------------------------------------------------------------
	typedef BOOL(WINAPI* LPFN_GLPIex)(
		LOGICAL_PROCESSOR_RELATIONSHIP,
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX,
		PDWORD);

	//--------------------------------------------------------------------------------------
	// Helper function to count set bits in the processor mask.	
	// ( Copied from the doc page for GetLogicalProcessorInformation() )
	//--------------------------------------------------------------------------------------
	DWORD CountSetBits(ULONG_PTR bitMask)
	{
		DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
		DWORD bitSetCount = 0;
		ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;
		DWORD i;

		for (i = 0; i <= LSHIFT; ++i)
		{
			bitSetCount += ((bitMask & bitTest) ? 1 : 0);
			bitTest /= 2;
		}

		return bitSetCount;
	}

	void GetCPUVendorString(std::string& cpuVendorString)
	{
		int cpuInfo[4] = { -1 };
		char vendor[0x10];
		memset(vendor, 0, sizeof(vendor));

		__cpuid(cpuInfo, 0);
		*reinterpret_cast<int*>(vendor) = cpuInfo[1];
		*reinterpret_cast<int*>(vendor + 4) = cpuInfo[3];
		*reinterpret_cast<int*>(vendor + 8) = cpuInfo[2];

		cpuVendorString = vendor;
	}

	void GetCPUBrandString(std::string& cpuBrandString)
	{
		int nExIds;
		int cpuInfo[4] = { -1 };
		char brand[0x40];
		memset(brand, 0, sizeof(brand));

		__cpuid(cpuInfo, 0x80000000);
		nExIds = cpuInfo[0];

		// Interpret CPU brand string if reported
		if (nExIds >= 0x80000004)
		{
			__cpuidex(cpuInfo, 0x80000002, 0);
			memcpy(brand, cpuInfo, sizeof(cpuInfo));

			__cpuidex(cpuInfo, 0x80000003, 0);
			memcpy(brand + 16, cpuInfo, sizeof(cpuInfo));

			__cpuidex(cpuInfo, 0x80000004, 0);
			memcpy(brand + 32, cpuInfo, sizeof(cpuInfo));
		}

		cpuBrandString = brand;
	}

	int GetCoreInfo(CPUData::CoreInfo& coreInfo)
	{
		LPFN_GLPIex glpi;
		BOOL done = FALSE;
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX  buffer = NULL;
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX  ptr = NULL;
		DWORD returnLength = 0;
		DWORD byteOffset = 0;
		DWORD numProcessorCores = 0;
		DWORD numBigCores = 0;
		DWORD numSmallCores = 0;
		DWORD numLP = 0;
		DWORD numBigLP = 0;
		DWORD numSmallLP = 0;
		DWORD_PTR maskLP = 0;
		DWORD_PTR maskBigLP = 0;
		DWORD_PTR maskSmallLP = 0;

		glpi = (LPFN_GLPIex)GetProcAddress(
			GetModuleHandle(TEXT("kernel32")),
			"GetLogicalProcessorInformationEx");
		if (NULL == glpi)
		{
			printf("\GetLogicalProcessorInformationEx is not supported.\n");
			return (1);
		}

		while (!done)
		{
			DWORD rc = glpi(RelationProcessorCore, buffer, &returnLength);

			if (FALSE == rc)
			{
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
				{
					if (buffer)
						free(buffer);

					buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)malloc(
						returnLength);

					if (NULL == buffer)
					{
						printf("\nError: Allocation failure\n");
						return (2);
					}
				}
				else
				{
					printf("\nError %d\n", GetLastError());
					return (3);
				}
			}
			else
			{
				done = TRUE;
			}
		}

		ptr = buffer;

		while (byteOffset < returnLength)
		{
#if defined(DEBUG) | defined(_DEBUG) 
			printf("Core No.: %d\n", numProcessorCores);
			printf("HT/SMT status: %d\n", ptr->Processor.Flags);
			printf("EfficiencyClass: %d\n\n", ptr->Processor.EfficiencyClass);
#endif
			// If the PROCESSOR_RELATIONSHIP is RelationProcessorCore, the GroupCount member is always 1.
			DWORD_PTR mask = ptr->Processor.GroupMask[0].Mask;
			unsigned int count = CountSetBits(mask);

			numProcessorCores++;

			// A hyperthreaded core supplies more than one logical processor.
			numLP += count;
			maskLP |= mask;

			//On systems with a homogeneous set of cores, the value of EfficiencyClass is always 0.
			//On Intel hybrid CPU, the value of EfficiencyClass is greater than 0 for big cores, and is 0 for small cores. 
			if (ptr->Processor.EfficiencyClass > 0)
			{
				blog_info("big core");
				numBigCores++;
				numBigLP += count;
				maskBigLP |= mask;
			}
			else
			{
				blog_info("small core");
				numSmallCores++;
				numSmallLP += count;
				maskSmallLP |= mask;
			}

			byteOffset += ptr->Size;
			ptr = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX)((byte*)buffer + byteOffset);
		}

		free(buffer);

		//For systems with a homogeneous set of cores
		if (numBigCores == 0)
		{
			numSmallCores = 0;
			numSmallLP = 0;
			maskSmallLP = 0;
		}

		coreInfo.processorCoreCount = numProcessorCores;
		coreInfo.bigCoreCount = numBigCores;
		coreInfo.smallCoreCount = numSmallCores;
		coreInfo.logicalProcessorCount = numLP;
		coreInfo.bigLogicalProcessorCount = numBigLP;
		coreInfo.smallLogicalProcessorCount = numSmallLP;
		coreInfo.logicalProcessorMask = maskLP;
		coreInfo.bigLogicalProcessorMask = maskBigLP;
		coreInfo.smallLogicalProcessorMask = maskSmallLP;
		blog_info("ProcessorCore:%d, %d, %d", numProcessorCores, numBigCores, numSmallCores);
		blog_info("ProcessorCount:%d, %d, %d", numLP, numBigLP, numSmallLP);
		blog_info("ProcessorMask:(0x%016X), (0x%016X), (0x%016X)", maskLP, maskBigLP, maskSmallLP);
		return 0;
	}

	int InitCPUInfo(CPUData& cpuData)
	{
		int returnCode = 0;

		GetCPUVendorString(cpuData.CPUVendor);

		GetCPUBrandString(cpuData.CPUBrand);

		returnCode = GetCoreInfo(cpuData.coreInfo);

		cpuData.isHybrid = (cpuData.coreInfo.bigCoreCount) ? true : false;

		return returnCode;
	}

	bool IsIntelCPU()
	{
		std::string CPUVendor;

		GetCPUVendorString(CPUVendor);

		return CPUVendor == "GenuineIntel";
	}
}
