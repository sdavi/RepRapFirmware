//Author: sdavi

#include "chip.h" //LPC Open
#include "DMA.h"

//ESP connected to SSP0

//maintain compatibility with RRF code
void spi_disable(SSPChannel sspChannel) noexcept
{
    //Just turn off DMA
    spi_dma_disable();
}

static inline void flush_rx_fifo() noexcept
{
    while(LPC_SSP0->SR & (1UL << 2))
    {
        (void)LPC_SSP0->DR;
    }
}

static inline void spi_rx_dma_enable() noexcept
{
    LPC_SSP0->DMACR |= SSP_DMA_RX;//enable RX DMA
}

static inline void spi_tx_dma_enable() noexcept
{
    LPC_SSP0->DMACR |= SSP_DMA_TX;//enable TX DMA
}

static inline void spi_rx_dma_disable() noexcept
{
    LPC_SSP0->DMACR &= ~SSP_DMA_RX;
}

static inline void spi_tx_dma_disable() noexcept
{
    LPC_SSP0->DMACR &= ~SSP_DMA_TX;
}

static inline void spi_dma_disable() noexcept
{
	spi_tx_dma_disable();
	spi_rx_dma_disable();
}

static inline void spi_dma_enable() noexcept
{
    spi_rx_dma_enable();
    spi_tx_dma_enable();
}

static bool spi_dma_check_rx_complete() noexcept
{
    //DMA Interrupt will notifiy when transfer is complete, just return true here
    return true;
}

static void spi_tx_dma_setup(const void *buf, uint32_t transferLength) noexcept
{
    // Setup DMA transfer: outBuffer --> SSP0 (Memory to Peripheral Transfer)
    SspDmaTxTransfer(DMA_SSP0_TX, buf, transferLength);
}

static void spi_rx_dma_setup(const void *buf, uint32_t transferLength) noexcept
{
    // Setup DMA Receive: SSP0 --> inBuffer (Peripheral to Memory)
    SspDmaRxTransfer(DMA_SSP0_RX, buf, transferLength);
}

static void spi_slave_dma_setup(uint32_t dataOutSize, uint32_t dataInSize) noexcept
{
    flush_rx_fifo(); //flush SPI
    spi_dma_enable();

    //Find the largest transfer size
    //We will use the RX DMA complete interrupt to notify when the transfer is complete.
    const uint32_t dsize = MAX(dataOutSize, dataInSize) + sizeof(MessageHeaderSamToEsp);
    spi_tx_dma_setup(&bufferOut, dsize);
    spi_rx_dma_setup(&bufferIn, dsize);

//    spi_tx_dma_setup(&bufferOut, dataOutSize + sizeof(MessageHeaderSamToEsp));
//    spi_rx_dma_setup(&bufferIn, dataInSize + sizeof(MessageHeaderEspToSam));
}

// SPI interrupt handlers
void ESP_SPI_HANDLER(void) noexcept
{
	wifiInterface->SpiInterrupt();
}


void WiFiInterface::SpiInterrupt() noexcept
{
    const uint32_t status = LPC_SSP0->SR;
    spi_dma_disable();

    if((status & (1<<3)) != 0) //(Slave Abort) is set when the Slave Select (SSEL) signal goes inactive before a data transfer completes. )
    {
        
    }
    if((status & (1<<5)) != 0) //(Read Overrun) is set when the SPI receives data before it's read buffer is empty.
    {
        ++spiRxOverruns;
    }
    if((status & (1<<6)) != 0) //(Write Collision) is set when data is written to the SPI data register while a SPI data transfer is in progress.
    {
        ++spiTxUnderruns;
    }

    //pinMode(P1_21, OUTPUT_HIGH); //turn on LED3
    digitalWrite(SamTfrReadyPin, LOW);

    transferPending = false;
    TaskBase::GiveFromISR(espWaitingTask);
}


// Set up the SPI system
void WiFiInterface::SetupSpi() noexcept
{
    Chip_SSP_Disable(LPC_SSP0);

    Chip_Clock_SetPCLKDiv(SYSCTL_PCLK_SSP0, SYSCTL_CLKDIV_1); //set SPP0 peripheral clock to PCLK/1
    Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SSP0); //enable power and clocking

    //Setup pins for SSP0 to pinsel func2
    Chip_IOCON_PinMux(LPC_IOCON, 0, 15, IOCON_MODE_INACT, IOCON_FUNC2);
    Chip_IOCON_PinMux(LPC_IOCON, 0, 16, IOCON_MODE_INACT, IOCON_FUNC2);
    Chip_IOCON_PinMux(LPC_IOCON, 0, 17, IOCON_MODE_INACT, IOCON_FUNC2);
    Chip_IOCON_PinMux(LPC_IOCON, 0, 18, IOCON_MODE_INACT, IOCON_FUNC2);
    
    //LPC manual mentions that if CPHA is 0 then the CS needs to be pulsed between each byte(when in 8 bit mode).
    //Therefore if CS is held low during the entire transfer, we need to use a mode where CPHA = 1 (i.e. Mode 1 or Mode 3)
    Chip_SSP_SetFormat(LPC_SSP0, SSP_BITS_8, SSP_FRAMEFORMAT_SPI, SSP_CLOCK_MODE1);
    Chip_SSP_SetBitRate(LPC_SSP0, SystemCoreClock/2);
    Chip_SSP_Set_Mode(LPC_SSP0, SSP_MODE_SLAVE);
    
    //In Slave mode, the SSP clock rate provided by the master must not exceed 1/12 of the
    //SSP peripheral clock (which is set to PCLK/1 above), therefore to cater for LPC1768
    //(100MHz) max Master SCK is limited to 8.33MHz

    Chip_SSP_Enable(LPC_SSP0);
    spi_dma_disable();

    //Setup DMA
    InitialiseDMA();
    AttachDMAChannelInterruptHandler(ESP_SPI_HANDLER, DMA_SSP0_RX); //attach to the RX complete DMA Interrupt handler
}
    
