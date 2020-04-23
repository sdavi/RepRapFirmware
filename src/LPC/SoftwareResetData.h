//SD

#ifndef SOFTWARERESETDATA_H_
#define SOFTWARERESETDATA_H_

#include "Core.h"
#include "iap.h"


bool LPC_IsSoftwareResetDataSlotVacant(uint8_t slot) noexcept;
uint32_t *LPC_GetSoftwareResetDataSlotPtr(uint8_t slot) noexcept;
void LPC_ReadSoftwareResetDataSlot(uint8_t slot, void *data, uint32_t dataLength) noexcept;
bool LPC_EraseSoftwareResetDataSlots() noexcept;
bool LPC_WriteSoftwareResetData(uint8_t slot, const void *data, uint32_t dataLength) noexcept;

//Compatibility
inline void EraseAndReset() noexcept {};


#endif /* SOFTWARERESETDATA_H_ */
