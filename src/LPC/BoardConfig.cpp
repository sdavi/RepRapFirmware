/*
 * Board Config
 *
 *  Created on: 3 Feb 2019
 *      Author: sdavi
 */



#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>

#include "BoardConfig.h"
#include "RepRapFirmware.h"
#include "GCodes/GCodeResult.h"
#include "sd_mmc.h"
#include "SharedSPI.h"

#include "Platform.h"

#include "SoftwarePWM.h"
#include "ff.h"

//Single entry for Board name
static const boardConfigEntry_t boardEntryConfig[]=
{
    {"lpc.board", &lpcBoardName, nullptr, cvStringType},
};

//All other board configs
static const boardConfigEntry_t boardConfigs[]=
{
    {"leds.diagnostic", &DiagPin, nullptr, cvPinType},

    //Steppers
    {"stepper.enablePins", ENABLE_PINS, &MaxTotalDrivers, cvPinType},
    {"stepper.stepPins", STEP_PINS, &MaxTotalDrivers, cvPinType},
    {"stepper.directionPins", DIRECTION_PINS, &MaxTotalDrivers, cvPinType},
    {"stepper.digipotFactor", &digipotFactor, nullptr, cvFloatType},

    //Heater sensors
    {"heat.tempSensePins", TEMP_SENSE_PINS, &NumThermistorInputs, cvPinType},
    {"heat.spiTempSensorCSPins", SpiTempSensorCsPins, &MaxSpiTempSensors, cvPinType},
    {"heat.spiTempSensorChannel", &TempSensorSSPChannel, nullptr, cvUint8Type},
    
    //ATX Power
    {"atx.powerPin", &ATX_POWER_PIN, nullptr, cvPinType},
    {"atx.powerPinInverted", &ATX_POWER_INVERTED, nullptr, cvBoolType},

    //SDCards
    {"sdCard.internal.spiFrequencyHz", &InternalSDCardFrequency, nullptr, cvUint32Type},
    {"sdCard.external.csPin", &SdSpiCSPins[1], nullptr, cvPinType},
    {"sdCard.external.cardDetectPin", &SdCardDetectPins[1], nullptr, cvPinType},
    {"sdCard.external.spiFrequencyHz", &ExternalSDCardFrequency, nullptr, cvUint32Type},
    {"sdCard.external.spiChannel", &ExternalSDCardSSPChannel, nullptr, cvUint8Type},

#if SUPPORT_12864_LCD
    {"lcd.lcdCSPin", &LcdCSPin, nullptr, cvPinType},
    {"lcd.lcdBeepPin", &LcdBeepPin, nullptr, cvPinType},
    {"lcd.encoderPinA", &EncoderPinA, nullptr, cvPinType},
    {"lcd.encoderPinB", &EncoderPinB, nullptr, cvPinType},
    {"lcd.encoderPinSw", &EncoderPinSw, nullptr, cvPinType},
    {"lcd.lcdDCPin", &LcdDCPin, nullptr, cvPinType},
    {"lcd.panelButtonPin", &PanelButtonPin, nullptr, cvPinType},
    {"lcd.spiChannel", &LcdSpiChannel, nullptr, cvUint8Type},
#endif
    
    {"softwareSPI.pins", SoftwareSPIPins, &NumSoftwareSPIPins, cvPinType}, //SCK, MISO, MOSI
    
#if HAS_WIFI_NETWORKING
    {"8266wifi.espDataReadyPin", &EspDataReadyPin, nullptr, cvPinType},
    {"8266wifi.lpcTfrReadyPin", &SamTfrReadyPin, nullptr, cvPinType},
    {"8266wifi.espResetPin", &EspResetPin, nullptr, cvPinType},
    {"8266wifi.serialRxTxPins", &WifiSerialRxTxPins, &NumberSerialPins, cvPinType},
#endif

#if HAS_LINUX_INTERFACE
    {"sbc.lpcTfrReadyPin", &LinuxTfrReadyPin, nullptr, cvPinType},
#endif

#if defined(SERIAL_AUX_DEVICE)
    {"serial.aux.rxTxPins", &AuxSerialRxTxPins, &NumberSerialPins, cvPinType},
#endif
#if defined(SERIAL_AUX2_DEVICE)
    {"serial.aux2.rxTxPins", &Aux2SerialRxTxPins, &NumberSerialPins, cvPinType},
#endif
    
    {"adc.prefilter.enable", &ADCEnablePreFilter, nullptr, cvBoolType},
};

#if !HAS_MASS_STORAGE
// Provide dummy functions for locks etc. for ff when mass storage is disabled

extern "C"
{
    // Create a sync object. We already created it, we just need to copy the handle.
    int ff_cre_syncobj (BYTE vol, FF_SYNC_t* psy)
    {
        return 1;
    }

    // Lock sync object
    int ff_req_grant (FF_SYNC_t sy)
    {
        return 1;
    }

    // Unlock sync object
    void ff_rel_grant (FF_SYNC_t sy)
    {
    }

    // Delete a sync object
    int ff_del_syncobj (FF_SYNC_t sy)
    {
        return 1;        // nothing to do, we never delete the mutex
    }
}
#endif

static inline bool isSpaceOrTab(char c) noexcept
{
    return (c == ' ' || c == '\t');
}
    
BoardConfig::BoardConfig() noexcept
{
    
}


void BoardConfig::Init() noexcept
{

    constexpr char boardConfigPath[] = "0:/sys/board.txt";
    FIL configFile;
    FATFS fs;
    FRESULT rslt;
    
        // Timer 0 is used for step generation (set elsewhere)
    NVIC_SetPriority(TIMER1_IRQn, 8);                       //Timer 1 is free
    NVIC_SetPriority(TIMER2_IRQn, NvicPriorityTimerServo);  //Timer 2 runs the PWM for Servos at 50hz
    NVIC_SetPriority(TIMER3_IRQn, 8);    //Timer 3 free
    NVIC_SetPriority(DMA_IRQn, NvicPrioritySpi);
    NVIC_SetPriority(PWM1_IRQn, NvicPriorityTimerPWM);  //HWPWM running in timer mode runs Software PWM
    
    NVIC_EnableIRQ(BOD_IRQn); // enable the BrownOut Detector interrupt
    
#if !HAS_MASS_STORAGE
    sd_mmc_init(SdWriteProtectPins, SdSpiCSPins);
#endif
    // Mount the internal SD card
    rslt = f_mount (&fs, "0:", 1);
    if (rslt == FR_OK)
    {
        //Open File
        rslt = f_open (&configFile, boardConfigPath, FA_READ);
        if (rslt != FR_OK)
        {
            delay(3000);        // Wait a few seconds so users have a chance to see this
            reprap.GetPlatform().MessageF(UsbMessage, "Unable to read board configuration: %s...\n",boardConfigPath );
            f_unmount ("0:");
            return;
        }
    }
    else
    {
        // failed to mount card
        delay(3000);        // Wait a few seconds so users have a chance to see this
        reprap.GetPlatform().MessageF(UsbMessage, "Failed to mount sd card\n");
        return;
    }
    if(rslt == FR_OK)
    {
            
        reprap.GetPlatform().MessageF(UsbMessage, "Loading LPC config from %s...\n", boardConfigPath );

        
        //First find the board entry to load the correct PinTable for looking up Pin by name
        BoardConfig::GetConfigKeys(&configFile, boardEntryConfig, (size_t) 1);
        if(!SetBoard(lpcBoardName)) // load the Correct PinTable for the defined Board (RRF3)
        {
            //Failed to find string in known boards array
            reprap.GetPlatform().MessageF(UsbMessage, "Unknown board: %s\n", lpcBoardName );
            SafeStrncpy(lpcBoardName, "generic", 8); //replace the string in lpcBoardName to "generic"
        }

        //Load all other config settings now that PinTable is loaded.
        f_lseek(&configFile, 0); //go back to beginning of config file
        BoardConfig::GetConfigKeys(&configFile, boardConfigs, (size_t) ARRAY_SIZE(boardConfigs));
        f_close(&configFile);
        f_unmount ("0:");
        
        //Calculate STEP_DRIVER_MASK (used for parallel writes)
        STEP_DRIVER_MASK = 0;
        for(size_t i=0; i<MaxTotalDrivers; i++)
        {
            //It is assumed all pins will be on Port 2
            const Pin stepPin = STEP_PINS[i];
            if( stepPin != NoPin && (stepPin >> 5) == 2) // divide by 32 to get port number
            {
                STEP_DRIVER_MASK |= (1 << (stepPin & 0x1f)); //this is a bitmask of all the stepper pins on Port2 used for Parallel Writes
            }
            else
            {
                if(stepPin != NoPin)
                {
                    // configured step pins are not on the same port - not using parallel writes
                    hasStepPinsOnDifferentPorts = true;
                }
            }
        }
        
        //Does board have build in current control via digipots?
        if(digipotFactor > 1)
        {
            hasDriverCurrentControl = true;
        }
        
        //Setup the Software SPI Pins
        sspi_setPinsForChannel(SWSPI0, SoftwareSPIPins[0], SoftwareSPIPins[1], SoftwareSPIPins[2]);

        //Internal SDCard SPI Frequency
        sd_mmc_reinit_slot(0, SdSpiCSPins[0], InternalSDCardFrequency);
        
        //Configure the External SDCard
        if(SdSpiCSPins[1] != NoPin)
        {
            setPullup(SdCardDetectPins[1], true);
            //set the SSP Channel for External SDCard
            if(ExternalSDCardSSPChannel == SSP0 || ExternalSDCardSSPChannel == SSP1)
            {
                sd_mmc_setSSPChannel(1, ExternalSDCardSSPChannel); //must be called before reinit
            }
            //set the CSPin and the frequency for the External SDCard
            sd_mmc_reinit_slot(1, SdSpiCSPins[1], ExternalSDCardFrequency);
        }
        
        #if HAS_WIFI_NETWORKING
            if(SamCsPin != NoPin) pinMode(SamCsPin, OUTPUT_LOW);
            if(EspResetPin != NoPin) pinMode(EspResetPin, OUTPUT_LOW);
            
            if(WifiSerialRxTxPins[0] != NoPin && WifiSerialRxTxPins[1] != NoPin)
            {
                //Setup the Serial Port for ESP Wifi
                APIN_Serial1_RXD = WifiSerialRxTxPins[0];
                APIN_Serial1_TXD = WifiSerialRxTxPins[1];
                
                if(!SERIAL_WIFI_DEVICE.Configure(WifiSerialRxTxPins[0], WifiSerialRxTxPins[1]))
                {
                    reprap.GetPlatform().MessageF(UsbMessage, "Failed to set WIFI Serial with pins %d.%d and %d.%d.\n", (WifiSerialRxTxPins[0] >> 5), (WifiSerialRxTxPins[0] & 0x1F), (WifiSerialRxTxPins[1] >> 5), (WifiSerialRxTxPins[1] & 0x1F) );
                }
            }
        #endif

        #if defined(SERIAL_AUX_DEVICE)
            //Configure Aux Serial
            if(AuxSerialRxTxPins[0] != NoPin && AuxSerialRxTxPins[1] != NoPin)
            {
                if(!SERIAL_AUX_DEVICE.Configure(AuxSerialRxTxPins[0], AuxSerialRxTxPins[1]))
                {
                    reprap.GetPlatform().MessageF(UsbMessage, "Failed to set AUX Serial with pins %d.%d and %d.%d.\n", (AuxSerialRxTxPins[0] >> 5), (AuxSerialRxTxPins[0] & 0x1F), (AuxSerialRxTxPins[1] >> 5), (AuxSerialRxTxPins[1] & 0x1F) );
                }

            }
        #endif

        #if defined(SERIAL_AUX2_DEVICE)
            //Configure Aux2 Serial
            if(Aux2SerialRxTxPins[0] != NoPin && Aux2SerialRxTxPins[1] != NoPin)
            {
                if(!SERIAL_AUX2_DEVICE.Configure(Aux2SerialRxTxPins[0], Aux2SerialRxTxPins[1]))
                {
                    reprap.GetPlatform().MessageF(UsbMessage, "Failed to set AUX2 Serial with pins %d.%d and %d.%d.\n", (Aux2SerialRxTxPins[0] >> 5), (Aux2SerialRxTxPins[0] & 0x1F), (Aux2SerialRxTxPins[1] >> 5), (Aux2SerialRxTxPins[1] & 0x1F) );
                }

            }
        #endif

        
        //Init pins for LCD
        //make sure to init ButtonPin as input incase user presses button
        if(PanelButtonPin != NoPin) pinMode(PanelButtonPin, INPUT); //unused
        if(LcdDCPin != NoPin) pinMode(LcdDCPin, OUTPUT_HIGH); //unused
        if(LcdBeepPin != NoPin) pinMode(LcdBeepPin, OUTPUT_LOW);
        // Set the 12864 display CS pin low to prevent it from receiving garbage due to other SPI traffic
        if(LcdCSPin != NoPin) pinMode(LcdCSPin, OUTPUT_LOW);

        //Init Diagnostcs Pin
        pinMode(DiagPin, OUTPUT_LOW);
        
        //Configure ADC pre filter
        ConfigureADCPreFilter(ADCEnablePreFilter);
    }
}


//Convert a pin string into a RRF Pin
//Handle formats such as 1.23, 1_23, P1_23 or P1.23
Pin BoardConfig::StringToPin(const char *strvalue) noexcept
{
    if(strvalue == nullptr) return NoPin;
    
    if(tolower(*strvalue) == 'p') strvalue++; //skip P
    
    //check size.. should be 3chars or 4 chars i.e. 0.1, 2.25, 1_23. 2nd char should be . or _
    uint8_t len = strlen(strvalue);
    if((len == 3 || len == 4) && (*(strvalue+1) == '.' || *(strvalue+1) == '_') )
    {
        const char *ptr = nullptr;
        
        uint8_t port = SafeStrtoul(strvalue, &ptr, 10);
        if(ptr > strvalue && port <= 4)
        {
            strvalue+=2;
            uint8_t pin = SafeStrtoul(strvalue, &ptr, 10);
            
            if(ptr > strvalue && pin < 32)
            {
                //Convert the Port and Pin to match the arrays in CoreLPC
                Pin lpcpin = (Pin) ( (port << 5) | pin);
                return lpcpin;
            }
        }
    }
    
    return NoPin;
}

Pin BoardConfig::LookupPin(char *strvalue) noexcept
{
    //Lookup a pin by name
    LogicalPin lp;
    bool hwInverted;
    
    //convert string to lower case for LookupPinName
    for(char *l = strvalue; *l; l++) *l = tolower(*l);
    
    if(LookupPinName(strvalue, lp, hwInverted))
    {
        return PinTable[lp].pin; //lookup succeeded, return the Pin
    }
                     
    //pin may not be in the pintable so check if the format is a correct pin (returns NoPin if not)
    return StringToPin(strvalue);
}



void BoardConfig::PrintValue(MessageType mtype, configValueType configType, void *variable) noexcept
{
    switch(configType)
    {
        case cvPinType:
            {
                Pin pin = *(Pin *)(variable);
                if(pin == NoPin)
                {
                    reprap.GetPlatform().MessageF(mtype, "NoPin ");
                }
                else
                {
                    reprap.GetPlatform().MessageF(mtype, "%d.%d ", (pin >> 5), (pin & 0x1F) );
                }
            }
            break;
        case cvBoolType:
            reprap.GetPlatform().MessageF(mtype, "%s ", (*(bool *)(variable) == true)?"true":"false" );
            break;
        case cvFloatType:
            reprap.GetPlatform().MessageF(mtype, "%.2f ",  (double) *(float *)(variable) );
            break;
        case cvUint8Type:
            reprap.GetPlatform().MessageF(mtype, "%u ",  *(uint8_t *)(variable) );
            break;
        case cvUint16Type:
            reprap.GetPlatform().MessageF(mtype, "%d ",  *(uint16_t *)(variable) );
            break;
        case cvUint32Type:
            reprap.GetPlatform().MessageF(mtype, "%lu ",  *(uint32_t *)(variable) );
            break;
        case cvStringType:
            reprap.GetPlatform().MessageF(mtype, "%s ",  (char *)(variable) );
            break;
        default:{
            
        }
    }
}


//Information printed by M122 P200
void BoardConfig::Diagnostics(MessageType mtype) noexcept
{
    reprap.GetPlatform().MessageF(mtype, "== Configurable Board.txt Settings ==\n");

    //Print the board name
    boardConfigEntry_t board = boardEntryConfig[0];
    reprap.GetPlatform().MessageF(mtype, "%s = ", board.key );
    BoardConfig::PrintValue(mtype, board.type, board.variable);
    reprap.GetPlatform().MessageF(mtype, "\n");

    
    //Print rest of board configurations
    const size_t numConfigs = ARRAY_SIZE(boardConfigs);
    for(size_t i=0; i<numConfigs; i++)
    {
        boardConfigEntry_t next = boardConfigs[i];

        reprap.GetPlatform().MessageF(mtype, "%s = ", next.key );
        if(next.maxArrayEntries != nullptr)
        {
            reprap.GetPlatform().MessageF(mtype, "{ ");
            for(size_t p=0; p<*(next.maxArrayEntries); p++)
            {
                //TODO:: handle other values
                if(next.type == cvPinType){
                    BoardConfig::PrintValue(mtype, next.type, (void *)&((Pin *)(next.variable))[p]);
                }
            }
            reprap.GetPlatform().MessageF(mtype, "}\n");
        }
        else
        {
            BoardConfig::PrintValue(mtype, next.type, next.variable);
            reprap.GetPlatform().MessageF(mtype, "\n");

        }
    }
    
#if defined(SERIAL_AUX_DEVICE) || defined(SERIAL_AUX2_DEVICE) || HAS_WIFI_NETWORKING
    reprap.GetPlatform().MessageF(mtype, "\n== Hardware Serial ==\n");
    #if defined(SERIAL_AUX_DEVICE)
        reprap.GetPlatform().MessageF(mtype, "AUX Serial: %s%c\n", ((SERIAL_AUX_DEVICE.GetUARTPortNumber() == -1)?"Disabled": "UART "), (SERIAL_AUX_DEVICE.GetUARTPortNumber() == -1)?' ': ('0' + SERIAL_AUX_DEVICE.GetUARTPortNumber()));
    #endif
    #if defined(SERIAL_AUX2_DEVICE)
        reprap.GetPlatform().MessageF(mtype, "AUX2 Serial: %s%c\n", ((SERIAL_AUX2_DEVICE.GetUARTPortNumber() == -1)?"Disabled": "UART "), (SERIAL_AUX2_DEVICE.GetUARTPortNumber() == -1)?' ': ('0' + SERIAL_AUX2_DEVICE.GetUARTPortNumber()));
    #endif
    #if HAS_WIFI_NETWORKING
        reprap.GetPlatform().MessageF(mtype, "WIFI Serial: %s%c\n", ((SERIAL_WIFI_DEVICE.GetUARTPortNumber() == -1)?"Disabled": "UART "), (SERIAL_WIFI_DEVICE.GetUARTPortNumber() == -1)?' ': ('0' + SERIAL_WIFI_DEVICE.GetUARTPortNumber()));
    #endif
#endif
    
    

    reprap.GetPlatform().MessageF(mtype, "\n== Software PWM ==\n");
    for(uint8_t i=0; i<MaxNumberSoftwarePWMPins; i++)
    {
        SoftwarePWM *next = softwarePWMEntries[i];
        if(next != nullptr)
        {
            const Pin pin = next->GetPin();
            reprap.GetPlatform().MessageF(mtype, "Pin %d.%d @ %dHz (%s)\n", (pin >> 5), (pin & 0x1f), next->GetFrequency(), next->IsRunning()?"Enabled":"Disabled" );
        }
    };
    
    //Print Servo PWM Timer
    reprap.GetPlatform().MessageF(mtype, "\n== Servo PWM ==\n");
    reprap.GetPlatform().MessageF(mtype, "Timer2 PWM = 50Hz ");
    PrintPinArray(mtype, Timer2PWMPins, MaxTimerEntries);
}

void BoardConfig::PrintPinArray(MessageType mtype, Pin arr[], uint16_t numEntries) noexcept
{
    reprap.GetPlatform().MessageF(mtype, "[ ");
    for(uint8_t i=0; i<numEntries; i++)
    {
        if(arr[i] != NoPin)
        {
            reprap.GetPlatform().MessageF(mtype, "%d.%d ", (arr[i]>>5), (arr[i] & 0x1f));
        }
        else
        {
            reprap.GetPlatform().MessageF(mtype, "NoPin ");
        }
        
    }
    reprap.GetPlatform().MessageF(mtype, "]\n");
}


//Set a variable from a string using the specified data type
void BoardConfig::SetValueFromString(configValueType type, void *variable, char *valuePtr) noexcept
{
    switch(type)
    {
        case cvPinType:
            *(Pin *)(variable) = LookupPin(valuePtr);
            break;
            
        case cvBoolType:
            {
                bool res = false;
                
                if(strlen(valuePtr) == 1)
                {
                    //check for 0 or 1
                    if(valuePtr[0] == '1') res = true;
                }
                else if(strlen(valuePtr) == 4 && StringEqualsIgnoreCase(valuePtr, "true"))
                {
                    res = true;
                }
                *(bool *)(variable) = res;
            }
            break;
            
        case cvFloatType:
            {
                const char *ptr = nullptr;
                *(float *)(variable) = SafeStrtof(valuePtr, &ptr);
            }
            break;
        case cvUint8Type:
            {
                const char *ptr = nullptr;
                uint8_t val = SafeStrtoul(valuePtr, &ptr, 10);
                if(val < 0) val = 0;
                if(val > 0xFF) val = 0xFF;
                
                *(uint8_t *)(variable) = val;
            }
            break;
        case cvUint16Type:
            {
                const char *ptr = nullptr;
                uint16_t val = SafeStrtoul(valuePtr, &ptr, 10);
                if(val < 0) val = 0;
                if(val > 0xFFFF) val = 0xFFFF;
                
                *(uint16_t *)(variable) = val;
                    
            }
            break;
        case cvUint32Type:
            {
                const char *ptr = nullptr;
                *(uint32_t *)(variable) = SafeStrtoul(valuePtr, &ptr, 10);
            }
            break;
            
        case cvStringType:
            {
                
                //TODO:: string Type only handles Board Name variable
                if(strlen(valuePtr)+1 < MaxBoardNameLength)
                {
                    strcpy((char *)(variable), valuePtr);
                }
            }
            break;
            
        default:
            debugPrintf("Unhandled ValueType\n");
    }
}

static size_t ReadLine(FIL *fp, char *p, int len)
{
    int nc = 0;
    UINT rc;
    uint8_t s;
    len -= 1;    /* Make a room for the terminator */
    while (nc < len)
    {
        f_read(fp, &s, 1, &rc);
        if (rc != 1)
        {
            if (nc == 0) return -1;
            break;
        }
        if (s == '\r') continue;
        if (s == '\n') break;
        *p++ = s; nc++;
    }
    *p = 0;        /* Terminate the string */
    return nc;
}

bool BoardConfig::GetConfigKeys(FIL *configFile, const boardConfigEntry_t *boardConfigEntryArray, const size_t numConfigs) noexcept
{
    constexpr size_t maxLineLength = 120;
    char line[maxLineLength];

    int readLen = ReadLine(configFile, line, maxLineLength);
    while(readLen >= 0)
    {
        size_t len = (size_t) readLen;
        size_t pos = 0;
        while(pos < len && isSpaceOrTab(line[pos] && line[pos] != 0) == true) pos++; //eat leading whitespace

        if(pos < len){

            //check for comments
            if(line[pos] == '/' || line[pos] == '#')
            {
                //Comment - Skipping
            }
            else
            {
                const char* key = line + pos;
                while(pos < len && !isSpaceOrTab(line[pos]) && line[pos] != '=' && line[pos] != 0) pos++;
                line[pos] = 0;// null terminate the string (now contains the "key")

                pos++;

                //eat whitespace and = if needed
                while(pos < maxLineLength && line[pos] != 0 && (isSpaceOrTab(line[pos]) == true || line[pos] == '=') ) pos++; //skip spaces and =

                //debugPrintf("Key: %s", key);

                if(pos < len && line[pos] == '{')
                {
                    // { indicates the start of an array
                    
                    pos++; //skip the {

                    //Array of Values:
                    //TODO:: only Pin arrays are currently implemented
                    
                    //const size_t numConfigs = ARRAY_SIZE(boardConfigs);
                    for(size_t i=0; i<numConfigs; i++)
                    {
                        boardConfigEntry_t next = boardConfigEntryArray[i];
                        //Currently only handles Arrays of Pins
                        
                        
                        if(next.maxArrayEntries != nullptr /*&& next.type == cvPinType*/ && StringEqualsIgnoreCase(key, next.key))
                        {
                            //matched an entry in boardConfigEntryArray

                            //create a temp array to read into. Only copy the array entries into the final destination when we know the array is properly defined
                            const size_t maxArraySize = *next.maxArrayEntries;
                            
                            //Pin Array Type
                            Pin readArray[maxArraySize];

                            //eat whitespace
                            while(pos < maxLineLength && line[pos] != 0 && isSpaceOrTab(line[pos]) == true ) pos++;

                            bool searching = true;

                            size_t arrIdx = 0;

                            //search for values in Array
                            while( searching )
                            {
                                if(pos < maxLineLength)
                                {

                                    while(pos < maxLineLength && (isSpaceOrTab(line[pos]) == true)) pos++; // eat whitespace

                                    if(pos == maxLineLength)
                                    {
                                        debugPrintf("Got to end of line before end of array, line must be longer than maxLineLength");
                                        searching = false;
                                        break;
                                    }

                                    bool closedSuccessfully = false;
                                    //check brace isnt closed
                                    if(pos < maxLineLength && line[pos] == '}')
                                    {
                                        closedSuccessfully = true;
                                        arrIdx--; // we got the closing brace before getting a value this round, decrement arrIdx
                                    }
                                    else
                                    {

                                        if(arrIdx >= maxArraySize )
                                        {
                                            debugPrintf("Error : Too many entries defined in config for array\n");
                                            searching = false;
                                            break;
                                        }

                                        //Try to Read the next Value

                                        //should be at first char of value now
                                        char *valuePtr = line+pos;

                                        //read until end condition - space,comma,}  or null / # ;
                                        while(pos < maxLineLength && line[pos] != 0 && !isSpaceOrTab(line[pos]) && line[pos] != ',' && line[pos] != '}' && line[pos] != '/' && line[pos] != '#' && line[pos] != ';')
                                        {
                                            pos++;
                                        }

                                        //see if we ended due to comment, ;, or null
                                        if(pos == maxLineLength || line[pos] == 0 || line[pos] == '/' || line[pos] == '#' || line[pos]==';')
                                        {
                                            debugPrintf("Error: Array ended without Closing Brace?\n");
                                            searching = false;
                                            break;
                                        }

                                        //check if there is a closing brace after value without any whitespace, before it gets overwritten with a null
                                        if(line[pos] == '}')
                                        {
                                            closedSuccessfully = true;
                                        }

                                        line[pos] = 0; // null terminate the string

                                        //debugPrintf("%s ", valuePtr);

                                        //Put into the Temp Array
                                        if(arrIdx >= 0 && arrIdx<maxArraySize)
                                        {
                                            readArray[arrIdx] = LookupPin(valuePtr);
                                            
                                            //TODO:: HANDLE OTHER VALUE TYPES??
                                            

                                        }
                                    }

                                    if(closedSuccessfully == true)
                                    {
                                        //Array Closed - Finished Searching
                                        if(arrIdx >= 0 && arrIdx < maxArraySize) //arrIndx will be -1 if closed before reading any values
                                        {
                                            //All values read successfully, copy temp array into Final destination
                                            //dest array may be larger, dont overrite the default values
                                            for(size_t i=0; i<(arrIdx+1); i++ )
                                            {
                                                ((Pin *)(next.variable))[i] = readArray[i];
                                            }
                                            //Success!
                                            searching = false;
                                            break;

                                        }
                                        //failed to set values
                                        searching = false;
                                        break;
                                    }
                                    arrIdx++;
                                    pos++;
                                }
                                else
                                {
                                    debugPrintf("Unable to find values for Key\n");
                                    searching = false;
                                    break;
                                }
                            }//end while(searching)
                        }//end if matched key
                    }//end for

                }
                else
                {
                    //single value
                    if(pos < maxLineLength && line[pos] != 0)
                    {
                        //should be at first char of value now
                        char *valuePtr = line+pos;

                        //read until end condition - space, ;, comment, null,etc
                        while(pos < maxLineLength && line[pos] != 0 && !isSpaceOrTab(line[pos]) && line[pos] != ';' && line[pos] != '/') pos++;

                        //overrite the end condition with null....
                        line[pos] = 0; // null terminate the string (the "value")

                        //Find the entry in boardConfigEntryArray using the key
                        //const size_t numConfigs = ARRAY_SIZE(boardConfigs);
                        for(size_t i=0; i<numConfigs; i++)
                        {
                            boardConfigEntry_t next = boardConfigEntryArray[i];
                            //Single Value config entries have nullptr for maxArrayEntries
                            if(next.maxArrayEntries == nullptr && StringEqualsIgnoreCase(key, next.key))
                            {
                                //match
                                BoardConfig::SetValueFromString(next.type, next.variable, valuePtr);
                            }
                        }
                    }
                }
            }
        }
        else
        {
            //Empty Line - Nothing to do here
        }

        readLen = ReadLine(configFile, line, maxLineLength); //attempt to read the next line
        //debugPrintf("ReadLine returns %d\n", readLen);
    }
    return false;
}
