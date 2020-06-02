/*
 * SoftwareReset.cpp
 *
 *  Created on: 15 Nov 2019
 *      Author: David
 */

#include "SoftwareReset.h"
#include "Tasks.h"

extern uint32_t _estack;			// defined in the linker script

// The following must be kept in line with enum class SoftwareResetReason
const char *const SoftwareResetData::ReasonText[] =
{
	"User",
	"Erase",
	"NMI",
	"Hard fault",
	"Stuck in spin loop",
	"Watchdog timeout",
	"Usage fault",
	"Other fault",
	"Stack overflow",
	"Assertion failed",
	"Heat task stuck",
	"Memory protection fault",
	"Terminate called",
	"Pure virtual function called",
	"Deleted virtual function called",
	"Unknown"
};

uint8_t SoftwareResetData::extraDebugInfo;			// extra info for debugging

// Return true if this struct can be written without erasing it first
bool SoftwareResetData::isVacant() const noexcept
{
	const uint32_t *p = reinterpret_cast<const uint32_t*>(this);
	for (size_t i = 0; i < sizeof(*this)/sizeof(uint32_t); ++i)
	{
		if (*p != 0xFFFFFFFF)
		{
			return false;
		}
		++p;
	}
	return true;
}

#if defined(__LPC17xx__)
    extern "C"
    {
        volatile StackType_t *CheckSPCurrentTaskStack(const uint32_t *stackPointer); //defined in freertos_tasks_c_additions.h
    }
#endif
// Populate this reset data from the parameters passed and the CPU state
void SoftwareResetData::Populate(uint16_t reason, uint32_t time, const uint32_t *stk) noexcept
{
	magic = SoftwareResetData::magicValue;
	resetReason = reason | ((extraDebugInfo & 0x07) << 5);
	when = time;
	neverUsedRam = Tasks::GetNeverUsedRam();
	hfsr = SCB->HFSR;
	cfsr = SCB->CFSR;
	icsr = SCB->ICSR;
#if USE_MPU
	if ((reason & (uint16_t)SoftwareResetReason::mainReasonMask) == (uint16_t)SoftwareResetReason::memFault)
	{
		bfar = SCB->MMFAR;				// on a memory fault we store the MMFAR instead of the BFAR
	}
	else
	{
		bfar = SCB->BFAR;
	}
#else
	bfar = SCB->BFAR;
#endif
	// Get the task name if we can. There may be no task executing, so we must allow for this.
	const TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
	taskName = (currentTask == nullptr) ? 0 : *reinterpret_cast<const uint32_t*>(pcTaskGetName(currentTask));

#if defined(__LPC17xx__)
    //Find the highest address of the stack currently in use.
    
    volatile uint32_t *stackEnd = &_estack; //default to the exception stack

    if(currentTask != nullptr)
    {
        //Each task has its own stack which is currently statically assigned somewhere in RAM or AHB RAM
        //If the stack pointer is in the address range of the current task stack, stop at the high address of the task stack
        volatile uint32_t* taskStackHighAddress = CheckSPCurrentTaskStack(stk); //returns null if sp is not within the current task stack space
        if(taskStackHighAddress != nullptr)
        {
            stackEnd = taskStackHighAddress;
        }
    }
    
    if (stk != nullptr)
    {
        sp = reinterpret_cast<uint32_t>(stk);
        for (uint32_t& stval : stack)
        {
            stval = (stk < stackEnd) ? *stk : 0xFFFFFFFF;
            ++stk;
        }
    }
#else
    if (stk != nullptr)
	{
		sp = reinterpret_cast<uint32_t>(stk);
		for (uint32_t& stval : stack)
		{
			stval = (stk < &_estack) ? *stk : 0xFFFFFFFF;
			++stk;
		}
	}
#endif
}

// End
