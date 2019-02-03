#ifndef AZSMZ_H__
#define AZSMZ_H__


//Config for AZSMZ v1.0 (edited by AlexY)
//SD:: Fixes and updates for V2


//NOTES:
// Filament detector pin and Fan RPM pin must be on a spare pin on Port0 or Port2 only (UNTESTED)
// Azteeg X5 (and maybe others) probe endstop pin is not an ADC pin, so only Digital is supported, or select another spare ADC capable pin
// Note. ADC inputs are NOT 5V tolerant


// Default board type
#define DEFAULT_BOARD_TYPE BoardType::Lpc
#define ELECTRONICS "AZSMZ Mini"
#define LPC_ELECTRONICS_STRING "AZSMZ Mini"
#define LPC_BOARD_STRING "AZSMZ"

#define AZSMZ
//120MHz CPU
#define VARIANT_MCK 120000000

constexpr Pin LED1 = P1_18;
constexpr Pin LED2 = P1_19;
constexpr Pin LED3 = P1_20;
constexpr Pin LED4 = P1_21;

constexpr Pin LED_PLAY = LED1; //No Play LED, use LED1




// The physical capabilities of the machine

// The number of drives in the machine, including X, Y, and Z plus extruder drives
const size_t DRIVES = 5;

constexpr size_t NumDirectDrivers = DRIVES;                // The maximum number of drives supported by the electronics
constexpr size_t MaxTotalDrivers = NumDirectDrivers;
constexpr size_t MaxSmartDrivers = 0;                // The maximum number of smart drivers


constexpr size_t NumEndstops = 3;                    // The number of inputs we have for endstops, filament sensors etc.
constexpr size_t NumHeaters = 2;                    // The number of heaters in the machine; 0 is the heated bed even if there isn't one
constexpr size_t NumThermistorInputs = 2;


const size_t MinAxes = 3;						// The minimum and default number of axes
const size_t MaxAxes = 5;						// The maximum number of movement axes in the machine, usually just X, Y and Z, <= DRIVES

const size_t MaxExtruders = DRIVES - MinAxes;	// The maximum number of extruders
const size_t MaxDriversPerAxis = 2;				// The maximum number of stepper drivers assigned to one axis


// The numbers of entries in each array must correspond with the values of DRIVES, AXES, or HEATERS. Set values to NoPin to flag unavailability.
// DRIVES
//                                              X      Y      Z     E1     E2
const Pin ENABLE_PINS[DRIVES] =             { P0_4, P0_10, P0_19, P0_21,  P4_29};
const Pin STEP_PINS[DRIVES] =               { P2_0,  P2_1,  P2_2,  P2_3,  P2_8};
const uint8_t STEP_PIN_PORT2_POS[DRIVES] =  { 0,     1,     2,     3,     8}; //SD: Used for calculating bitmap for stepping drivers (this is position of the pins on the port)
const uint32_t STEP_DRIVER_MASK =           0x0000010F; //SD: mask of the step pins on Port 2 used for writing to step pins in parallel
const Pin DIRECTION_PINS[DRIVES] =          { P0_5, P0_11, P0_20, P0_22,  P2_13};



// Endstops
// Note: RepRapFirmware only as a single endstop per axis
//       gcode defines if it is a max ("high end") or min ("low end")
//       endstop.  gcode also sets if it is active HIGH or LOW
//

//AZSMZ has X,Y,Z,Probe
const Pin END_STOP_PINS[NumEndstops] = { P1_24, P1_26, P1_28 };

#define HAS_DRIVER_CURRENT_CONTROL 0

// HEATERS - The bed is assumed to be the at index 0
//                                            Bed    H1
const Pin TEMP_SENSE_PINS[NumThermistorInputs] = { P0_25, P0_23 };
const Pin HEAT_ON_PINS[NumHeaters] = { P2_5, P2_4 };

// PWM -
//       The Hardware PWM channels ALL share the same Frequency,
//       we will use Hardware PWM for Hotends (on board which have the heater on a hardware PWM capable pin)
//       So for PWM at a different frequency to the Hotend PWM (250Hz) use the Timers to generate PWM
//       by setting the options below. If a HW PWM pin is defined below as a timer pin, it will use the timer instead of PWM
//       except if the requested freq by RRF does not match the fixed timer freq.

//       Set to {NoPin, NoPin, NoPin } if not used
//       Below is a list of HW PWM pins. There are only 6 channels, some pins share the same channel
//       P1_18  Channel 1
//       P1_20  Channel 2
//       P1_21  Channel 3
//       P1_23  Channel 4
//       P1_24  Channel 5
//       P1_26  Channel 6
//       P2_0   Channel 1
//       P2_1   Channel 2
//       P2_2   Channel 3
//       P2_3   Channel 4
//       P2_4   Channel 5
//       P2_5   Channel 6
//       P3_25  Channel 2
//       P3_26  Channel 3

//AZSMZ: Bed (Timer1), H0 (HW PWM), Fan1 (Timer3), Fan2(Timer3)

#define Timer1_PWM_Frequency 10 //For Bed heaters or other slow PWM (10Hz is what RRF defaults to be compatible with SSRs)
#define Timer2_PWM_Frequency 50 //For Servos
#define Timer3_PWM_Frequency 250 //For Hotends not on HW PWM

#define Timer1_PWMPins {P2_5, NoPin, NoPin } // Bed at 10Hz
#define Timer2_PWMPins {NoPin, NoPin , NoPin}
#define Timer3_PWMPins {P2_7, P0_26, NoPin} //H1 and Fan2 at 250Hz


// Default thermistor betas
const float BED_R25 = 100000.0;
const float BED_BETA = 3950.0;
const float BED_SHC = 0.0;
const float EXT_R25 = 100000.0;
const float EXT_BETA = 3950.0;
const float EXT_SHC = 0.0;

// Thermistor series resistor value in Ohms
const float THERMISTOR_SERIES_RS = 4700.0;

const size_t MaxSpiTempSensors = 1;
// Digital pins the 31855s have their select lines tied to
const Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { NoPin };
constexpr SSPChannel TempSensorSSPChannel = SSP0;

// Digital pin number that controls the ATX power on/off
const Pin ATX_POWER_PIN = NoPin;

//Note: AZSMZ uses pin P1_29 which is NOT an ADC pin. Use a spare if need Analog in, else use digital options for probe
const Pin Z_PROBE_PIN = P1_29;
const Pin Z_PROBE_MOD_PIN = NoPin; // Digital pin number to turn the IR LED on (high) or off (low)
constexpr Pin DiagPin = NoPin;

// Use a PWM capable pin
const size_t NUM_FANS = 2;
const Pin COOLING_FAN_PINS[NUM_FANS] = { P2_7, P0_26 }; 

// Firmware will attach a FALLING interrupt to this pin
// see FanInterrupt() in Platform.cpp
// SD:: Note: Only GPIO pins on Port0 and Port2 support this. If needed choose from spare pins (UNTESTED)
const Pin COOLING_FAN_RPM_PIN = NoPin;

//Pins defined to use for external interrupt. **Must** be a pin on Port0 or Port2.
//I.e. for Fan RPM, Filament Detection etc
// We limit this to 3 to save memory
#define EXTERNAL_INTERRUPT_PINS {NoPin, NoPin, NoPin}



//SD: Internal SDCard is on SSP1
//    MOSI, MISO, SCLK, CS
//    P0_9, P0_8, P0_7, P0_6

//SD:: 2nd SDCard can be connected to SSP0
//    MOSI, MISO, SCLK
//    P0_18 P0_17 P0_15

// SD cards
//sd:: Internal SD card is on SSP1
//NOTE::: Although this is 2nd in the List, SSP1 is Configured to be Slot0 in coreNG to be compatible with RRF
//default to supporting 2 card..... if need 1_23 then change CS no No pin

const size_t NumSdCards = 2;//Note: use 2 even if only using 1 (internal) card
const Pin SdCardDetectPins[NumSdCards] = { NoPin, P3_25 };
const Pin SdWriteProtectPins[NumSdCards] = { NoPin, NoPin };
const Pin SdSpiCSPins[NumSdCards] = { P0_6, P0_16 };// Internal, external. If need 0_16 pin, and no ext sd card set to NoPin Note:: ("slot" 0 in CORE is configured to be LCP SSP1 to match default RRF behaviour)


// Definition of which pins we allow to be controlled using M42

const Pin SpecialPinMap[] =
{
 NoPin
};


//SPI LCD Common Settings (Viki2.0 and RRD Full Graphic Smart Display
constexpr SSPChannel LcdSpiChannel = SSP0;     //SSP0 (MISO0, MOSI0, SCK0)
constexpr Pin LcdCSPin =       P1_22; //LCD Chip Select
constexpr Pin LcdDCPin =       P2_6;  //DataControl Pin (A0) if none used set to NoPin
constexpr Pin LcdBeepPin =     P1_30;
constexpr Pin EncoderPinA =    P4_28;
constexpr Pin EncoderPinB =    P1_27;
constexpr Pin EncoderPinSw =   P3_26; //click
constexpr Pin PanelButtonPin = NoPin; //Extra button on Viki and RRD Panels (reset/back etc configurable)


//VIKI2.0 Specific options
constexpr Pin VikiRedLedPin = NoPin;
constexpr Pin VikiBlueLedPin = NoPin;



#endif
