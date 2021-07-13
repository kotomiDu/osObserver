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
#pragma once

#include <windows.h>
#include <string>

namespace CPUDetect
{
	struct CPUData
	{
		/*******************************************************************************
		 * CPUVendor
		 *
		 *     The string of the CPU Vender.
		 *
		 *     This value is initialized by the GetCPUVendorString function.
		 *
		 ******************************************************************************/
		std::string CPUVendor;

		/*******************************************************************************
		 * CPUBrand
		 *
		 *     The string of the CPU Brand.
		 *
		 *     This value is initialized by the GetCPUBrandString function.
		 *
		 ******************************************************************************/
		std::string CPUBrand;

		/*******************************************************************************
		 * isHybrid
		 *
		 *     Is true if the CPU has a heterogeneous set of cores. Otherwise, this
		 *	   value is false.
		 *
		 *     This value is initialized by the InitCPUInfo function.
		 *
		 ******************************************************************************/
		bool isHybrid;

		struct CoreInfo
		{
			/*******************************************************************************
			 * processorCoreCount
			 *
			 *     The number of the CPU physical cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			unsigned int processorCoreCount;

			/*******************************************************************************
			 * bigCoreCount
			 *
			 *     The number of big physical cores in the hybrid CPU. The value is zero for
			 *	   the CPU that has a homogeneous set of cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			unsigned int bigCoreCount;

			/*******************************************************************************
			 * smallCoreCount
			 *
			 *     The number of small physical cores (e.g. Atoms) of the hybrid CPU.
			 *	   The value is zero for the CPU that has a homogeneous set of cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			unsigned int smallCoreCount;

			/*******************************************************************************
			 * logicalProcessorCount
			 *
			 *     The number of the CPU logical cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			unsigned int logicalProcessorCount;

			/*******************************************************************************
			 * bigLogicalProcessorCount
			 *
			 *     The number of the big logical cores. The value is zero for the CPU that
			 *	   has a homogeneous set of cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			unsigned int bigLogicalProcessorCount;

			/*******************************************************************************
			 * smallLogicalProcessorCount
			 *
			 *     The number of the samll logical cores. The value is zero for the CPU that
			 *	   has a homogeneous set of cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			unsigned int smallLogicalProcessorCount;

			/*******************************************************************************
			 * logicalProcessorMask
			 *
			 *     The mask of the CPU logical cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			DWORD_PTR logicalProcessorMask;

			/*******************************************************************************
			 * bigLogicalProcessorMask
			 *
			 *     The mask of the big logical cores. The value is zero for the CPU that
			 *	   has a homogeneous set of cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			DWORD_PTR bigLogicalProcessorMask;

			/*******************************************************************************
			 * smallLogicalProcessorMask
			 *
			 *     The mask of the small logical cores.	The value is zero for the CPU that
			 *	   has a homogeneous set of cores.
			 *
			 *     This value is initialized by the GetCoreInfo function.
			 *
			 ******************************************************************************/
			DWORD_PTR smallLogicalProcessorMask;

		} coreInfo;
	};

	/*******************************************************************************
	 * InitCPUInfo
	 *
	 *     Query useful CPU information for the thread pool of applications. Returns
	 *     0 if no error was encountered, otherwise returns an error code.
	 *
	 *     cpuData
	 *         The struct in which the information will be stored.
	 *
	 ******************************************************************************/
	int InitCPUInfo(CPUData& cpuData);

	/*******************************************************************************
	 * GetCPUBrandString
	 *
	 *     Query the CPU brand.
	 *
	 *     cpuBrandString
	 *         The string of the CPU brand.
	 *
	 ******************************************************************************/
	void GetCPUBrandString(std::string& cpuBrandString);

	/*******************************************************************************
	 * GetCPUVendorString
	 *
	 *     Query the CPU brand.
	 *
	 *     cpuBrandString
	 *         The string of the CPU brand.
	 *
	 ******************************************************************************/
	void GetCPUVendorString(std::string& cpuVendorString);

	/*******************************************************************************
	 * GetCoreInfo
	 *
	 *     Query the physica and logical core counts of the CPU. Returns 0 if no
	 *     error was encountered, otherwise returns an error code.
	 *
	 *     coreInfo
	 *         The struct in which the core information will be stored
	 *
	 ******************************************************************************/
	int GetCoreInfo(CPUData::CoreInfo& coreInfo);

	/*******************************************************************************
	 * IsIntelCPU
	 *
	 *     Is true if the CPU vender is Intel. If this value is false, the CPU is
	 *     from other vendors.
	 *
	 ******************************************************************************/
	bool IsIntelCPU();
}
