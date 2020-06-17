//Author: sdavi

#include "chip.h" //LPC Open
#include "DMA.h"
#include "HardwareSPI.h"

//ESP connected to SSP0

static const sspi_device espSSPConfig =
{
    NoPin, //not used
    false, //not used
    8,
    SPI_MODE_1,
    SystemCoreClock/2,
    SSP0
};


//maintain compatibility with RRF code
void spi_disable(SSPChannel sspChannel) noexcept
{
    spi_dma_disable();
    
    ssp0.FlushAll();
}

void spi_dma_disable() noexcept
{
    LPC_SSP0->DMACR &= ~SSP_DMA_TX;
    LPC_SSP0->DMACR &= ~SSP_DMA_RX;
}

static bool spi_dma_check_rx_complete() noexcept
{
    //DMA Interrupt will notifiy when transfer is complete, just return true here
    return true;
}

static void spi_slave_dma_setup(uint32_t dataOutSize, uint32_t dataInSize) noexcept
{
    ssp0.FlushAll();

    //Find the largest transfer size
    //We will use the RX DMA complete interrupt to notify when the transfer is complete.
    const uint32_t dsize = MAX(dataOutSize, dataInSize) + sizeof(MessageHeaderSamToEsp);
    
    ssp0.SspDmaTxTransfer(&bufferOut, dsize);
    ssp0.SspDmaRxTransfer(&bufferIn, dsize);
}

// SPI interrupt handlers
void ESP_SPI_HANDLER(bool error) noexcept
{
	wifiInterface->SpiInterrupt();
}


void WiFiInterface::SpiInterrupt() noexcept
{
    spi_disable(SSP0);

    const uint32_t status = LPC_SSP0->RIS; //note: raw register holds status info regardless if the int is enabled.

    if(status && SSP_RORRIS) //Rx Overrun
    {
        LPC_SSP0->ICR |= SSP_RORIC; //clear interrupt
        ++spiRxOverruns;
    }

    digitalWrite(SamTfrReadyPin, LOW);

    transferPending = false;
    TaskBase::GiveFromISR(espWaitingTask);
}


// Set up the SPI system
void WiFiInterface::SetupSpi() noexcept
{
    ssp0.setup_device(&espSSPConfig, false); //setup in slave mode
    spi_disable(SSP0);
    
    AttachDMAChannelInterruptHandler(ESP_SPI_HANDLER, DMA_SSP0_RX); //attach to the RX complete DMA Interrupt handler
}
    
