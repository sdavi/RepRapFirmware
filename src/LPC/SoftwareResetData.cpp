
//SD

#include "SoftwareResetData.h"
#include "RepRapFirmware.h"


/*
 
 On LPC17xx we will save SoftwareReset Data to the final sector. The last sector is 32K in size.
 Data must be written in 256 or 512 or 1024 or 4096 bytes.

 The LPC1768/9 doesn't have the page erase IAP command, so we have to use the whole sector
 with a min write of 256bytes = 128 slots...

 */


/* Last sector address */
#define START_ADDR_LAST_SECTOR  0x00078000

constexpr uint32_t IAP_PAGE_SIZE  = 256;

/* LAST SECTOR */
#define IAP_LAST_SECTOR         29


uint32_t *LPC_GetSoftwareResetDataSlotPtr(uint8_t slot) noexcept
{
    return (uint32_t *) (START_ADDR_LAST_SECTOR + (slot*IAP_PAGE_SIZE));
}


//When the Sector is erased, all the bits will be high
//This checks if the first 4 bytes are all high for the designated software reset slot
//the first 2 bytes of a used reset slot will have the magic number in it.
bool LPC_IsSoftwareResetDataSlotVacant(uint8_t slot) noexcept
{
    const uint32_t *p = (uint32_t *) (START_ADDR_LAST_SECTOR + (slot*IAP_PAGE_SIZE));
    
    for (size_t i = 0; i < IAP_PAGE_SIZE/sizeof(uint32_t); ++i)
    {
        if (*p != 0xFFFFFFFF)
        {
            return false;
        }
        ++p;
    }
    return true;
 }

void LPC_ReadSoftwareResetDataSlot(uint8_t slot, void *data, uint32_t dataLength) noexcept
{
    uint32_t *slotStartAddress = (uint32_t *) (START_ADDR_LAST_SECTOR + (slot*IAP_PAGE_SIZE));
    memcpy(data, slotStartAddress, dataLength);
}


//erases a page in flash for the SoftwareResetData slot
bool LPC_EraseSoftwareResetDataSlots() noexcept
{
    // interrupts will be disabled before these are called.
    //__disable_irq(); //disable Interrupts

    uint8_t iap_ret_code;
    bool ret = false;

    
    /* Prepare to write/erase the last sector */
    iap_ret_code = Chip_IAP_PreSectorForReadWrite(IAP_LAST_SECTOR, IAP_LAST_SECTOR);
    
    /* Error checking */
    if (iap_ret_code != IAP_CMD_SUCCESS) {
        debugPrintf("Chip_IAP_PreSectorForReadWrite() failed to execute, return code is: %x\r\n", iap_ret_code);
        ret = false;
    }
    else
    {

        /* Erase the last Sector */
        iap_ret_code = Chip_IAP_EraseSector(IAP_LAST_SECTOR,IAP_LAST_SECTOR);
        
        /* Error checking */
        if (iap_ret_code != IAP_CMD_SUCCESS) {
            debugPrintf("Chip_IAP_ErasePage() failed to execute, return code is: %x\r\n", iap_ret_code);
            ret =  false;
        }
        else
        {
            ret = true;
        }
    
    
    }
    //__enable_irq();
    
    return ret;
    
}

bool LPC_WriteSoftwareResetData(uint8_t slot, const void *data, uint32_t dataLength) noexcept
{

    
    uint8_t iap_data_array[IAP_PAGE_SIZE];
    uint8_t iap_ret_code;
    bool ret = false;
    
    memset(iap_data_array, 0xFF, sizeof(iap_data_array)); //fill with FF
    //copy the data into our array (we must write 256 bytes)
    memcpy(iap_data_array, data, dataLength);
    //
    
    uint32_t slotStartAddress = (START_ADDR_LAST_SECTOR + (slot*IAP_PAGE_SIZE));
    
    //__disable_irq(); //disable Interrupts
    
    /* Prepare to write/erase the last sector */
    iap_ret_code = Chip_IAP_PreSectorForReadWrite(IAP_LAST_SECTOR, IAP_LAST_SECTOR);
    
    /* Error checking */
    if (iap_ret_code != IAP_CMD_SUCCESS) {
        debugPrintf("Chip_IAP_PreSectorForReadWrite() failed to execute, return code is: %x\r\n", iap_ret_code);
        ret = false;
    }
    else
    {
        //debugPrintf("About to write %d bytes to address %x", IAP_PAGE_SIZE, slotStartAddress);
        /* Write to the last sector */
        iap_ret_code = Chip_IAP_CopyRamToFlash(slotStartAddress, (uint32_t *)iap_data_array, IAP_PAGE_SIZE);

        /* Error checking */
        if (iap_ret_code != IAP_CMD_SUCCESS) {
            debugPrintf("Chip_IAP_CopyRamToFlash() failed to execute, return code is: %x\r\n", iap_ret_code);
            ret =  false;
        }
        else
        {
            ret = true;
        }
    }
    /* Re-enable interrupt mode */
    //__enable_irq();
    
    return ret;
    
}

