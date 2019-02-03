#ifndef SMOOTHIEBOARD_H__
#define SMOOTHIEBOARD_H__


//Config for Smoothieboard1


//NOTES:
// Filament detector pin and Fan RPM pin must be on a spare pin on Port0 or Port2 only (UNTESTED)
// Azteeg X5 (and maybe others) probe endstop pin is not an ADC pin, so only Digital is supported, or select another spare ADC capable pin
// Note. ADC inputs are NOT 5V tolerant
// Currently No support For Thermocouple etc sensors, only thermistors!


// Default board type
#define DEFAULT_BOARD_TYPE BoardType::Lpc
#define ELECTRONICS "Smoothieboard1"
#define LPC_ELECTRONICS_STRING "Smoothieboard1"
#define LPC_BOARD_STRING "SMOOTHIEBOARD"

//Firmware for networking version only. Non-networking firmware will need to be manually copied to Sdcard
#define FIRMWARE_FILE "firmware.bin"


#define SMOOTHIEBOARD1

//120MHz CPU
#define VARIANT_MCK 120000000

constexpr Pin LED1 = P1_18;
constexpr Pin LED2 = P1_19;
constexpr Pin LED3 = P1_20;
constexpr Pin LED4 = P1_21;

constexpr Pin LED_PLAY = LED1; //No Play LED, use LED1



// The physical capabilities of the machine

constexpr size_t DRIVES = 5; // The number of drives in the machine, including X, Y, and Z plus extruder drives

constexpr size_t NumDirectDrivers = DRIVES;                // The maximum number of drives supported by the electronics
constexpr size_t MaxTotalDrivers = NumDirectDrivers;
constexpr size_t MaxSmartDrivers = 0;                // The maximum number of smart drivers


constexpr size_t NumEndstops = 6;                    // The number of inputs we have for endstops, filament sensors etc.
constexpr size_t NumHeaters = 3;                    // The number of heaters in the machine; 0 is the heated bed even if there isn't one
constexpr size_t NumThermistorInputs = 3;

constexpr size_t MinAxes = 3;						// The minimum and default number of axes
constexpr size_t MaxAxes = 5;						// The maximum number of movement axes in the machine, usually just X, Y and Z, <= DRIVES

constexpr size_t MaxExtruders = DRIVES - MinAxes;	// The maximum number of extruders
constexpr size_t MaxDriversPerAxis = 2;				// The maximum number of stepper drivers assigned to one axis


// The numbers of entries in each array must correspond with the values of DRIVES, AXES, or HEATERS. Set values to NoPin to flag unavailability.
// DRIVES
//                                              X      Y      Z      E1     E2
constexpr Pin ENABLE_PINS[DRIVES] =             { P0_4,  P0_10, P0_19, P0_21,  P4_29  };
constexpr Pin STEP_PINS[DRIVES] =               { P2_0,  P2_1,  P2_2,  P2_3,   P2_8};
constexpr uint8_t STEP_PIN_PORT2_POS[DRIVES] =  { 0,     1,     2,     3,      8}; //SD: Used for calculating bitmap for stepping drivers (this is position of the pins on the port)
constexpr uint32_t STEP_DRIVER_MASK =           0x0000010F; //SD: mask of the step pins on Port 2 used for writing to step pins in parallel
constexpr Pin DIRECTION_PINS[DRIVES] =          { P0_5,  P0_11, P0_20, P0_22,  P2_13};



// Endstops
// RepRapFirmware only as a single endstop per axis
// gcode defines if it is a max ("high end") or min ("low end") endstop.  gcode also sets if it is active HIGH or LOW


//Smoothie has 6 Endstops
//                                          Xmin    Ymin  Zmin   Xmax   Ymax   Zmax
//                          RRF C Index     0       1     2      3      4      5
constexpr Pin END_STOP_PINS[NumEndstops] = {P1_24, P1_26, P1_28, P1_25, P1_27, P1_29};
#define LPC_MAX_MIN_ENDSTOPS 1
// Z Probe pin
// Default: Probe will be selected from an EndStop Pin (which are digital input only)
// Needs to be an ADC for certain modes, if needed then a spare A/D capable pin should be used and set Z_PROBE_PIN below.
// Must be an ADC capable pin.  Can be any of the ARM's A/D capable
// pins even a non-Arduino pin.

//Note: default to NoPin for Probe.
constexpr Pin Z_PROBE_PIN = NoPin;
// Digital pin number to turn the IR LED on (high) or off (low)
constexpr Pin Z_PROBE_MOD_PIN06 = NoPin;                                        // Digital pin number to turn the IR LED on (high) or off (low) on Duet v0.6 and v1.0 (PB21)
constexpr Pin Z_PROBE_MOD_PIN07 = NoPin;                                        // Digital pin number to turn the IR LED on (high) or off (low) on Duet v0.7 and v0.8.5 (PC10)
constexpr Pin Z_PROBE_MOD_PIN =   NoPin;


//Smoothie uses MCP4451

// Indices for motor current digipots (X,Y,Z,E) - E is on 2nd digipot chip
constexpr uint8_t POT_WIPES[5] = { 0, 1, 2, 3, 0};
constexpr float digipotFactor = 113.33; //factor for converting current to digipot value
#define HAS_DRIVER_CURRENT_CONTROL 1


// HEATERS - The bed is assumed to be the at index 0
//                                                     Bed    H1     H2
constexpr Pin TEMP_SENSE_PINS[NumThermistorInputs] = {P0_24, P0_23, P0_25};
constexpr Pin HEAT_ON_PINS[NumHeaters] = {P2_5, P2_7, P1_23};

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

//Smoothie: Bed (Timer1), H0 (Timer3), H1 (HW PWM), Fan1 (HWPWM), Fan2(Timer3)

#define Timer1_PWM_Frequency 10 //For Bed heaters or other slow PWM (10Hz is what RRF defaults to be compatible with SSRs)
#define Timer2_PWM_Frequency 50 //For Servos that dont like to run at faster frequencies
#define Timer3_PWM_Frequency 250 //For Hotends not on HW PWM

#define Timer1_PWMPins {P2_5, NoPin, NoPin }
#define Timer2_PWMPins {NoPin, NoPin , NoPin}
#define Timer3_PWMPins {P2_7, P2_6, NoPin}  


// Default thermistor betas
constexpr float BED_R25 = 100000.0;
constexpr float BED_BETA = 3988.0;
constexpr float BED_SHC = 0.0;
constexpr float EXT_R25 = 100000.0;
constexpr float EXT_BETA = 4388.0;
constexpr float EXT_SHC = 0.0;

// Thermistor series resistor value in Ohms
constexpr float THERMISTOR_SERIES_RS = 4700.0;

constexpr size_t MaxSpiTempSensors = 1;
// Digital pins the 31855s have their select lines tied to
constexpr Pin SpiTempSensorCsPins[MaxSpiTempSensors] = { NoPin };
constexpr SSPChannel TempSensorSSPChannel = SSP0;

// Digital pin number that controls the ATX power on/off
constexpr Pin ATX_POWER_PIN = NoPin;
constexpr Pin DiagPin = NoPin;


// Use a PWM capable pin
constexpr size_t NUM_FANS = 2;
constexpr Pin COOLING_FAN_PINS[NUM_FANS] = { P2_4, P2_6 }; //Note: P2_6 is not hardware PWMable!!

// Firmware will attach a FALLING interrupt to this pin
// see FanInterrupt() in Platform.cpp
// SD:: Note: Only GPIO pins on Port0 and Port2 support this. If needed choose from spare pins
//      Ensure to add this pin to the EXTERNAL_INTERRUPT_PINS below too
constexpr Pin COOLING_FAN_RPM_PIN = NoPin;


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
//default to supporting 2 card..... if need 0_28 then change CS no No pin

constexpr size_t NumSdCards = 2; //
constexpr Pin SdCardDetectPins[NumSdCards] = { NoPin, P0_27 };
constexpr Pin SdWriteProtectPins[NumSdCards] = { NoPin, NoPin };
constexpr Pin SdSpiCSPins[NumSdCards] = { P0_6, P0_28 };// Internal, external. Note:: ("slot" 0 in CORE is configured to be LCP SSP1 to match default RRF behaviour)
// Definition of which pins we allow to be controlled using M42




constexpr Pin SpecialPinMap[] =
{
    
    P1_24,  //XMin Endstop
    P1_26,  //yMin Endstop
    P0_26   //T4 (dont use AGND as a normal GND next to it when using as a Digital Pin
    
    //Spare on 3&4 driver boards
    //P1_22
    //P1_23 LED4
    //P4_29

};


//P0_17,    //SSP0(MISO)
//P0_16,    //CS LCD
//P0_15,    //SSP0(SCLK)
//P0_18,    //SSP0(MOSI)

//P0_27  // GLCD SD CardDetect
//P0_28  // GLCD SD ChipSelect

//P3_25 // GLCD encoder A
//P3_26 // GLCD encoder B
//P1_30 GLCD Click button
//P1_31 GLCD Buzzer
//P2_11 //GLCD pause/kill/back pin button


//SPI LCD Common Settings (RRD Full Graphic Smart Display)
constexpr SSPChannel LcdSpiChannel = SSP0;     //SSP0 (MISO0, MOSI0, SCK0)
constexpr Pin LcdCSPin =       P0_16; //LCD Chip Select
constexpr Pin LcdDCPin =       NoPin;  //DataControl Pin (A0) if none used set to NoPin
constexpr Pin LcdBeepPin =     P1_31;
constexpr Pin EncoderPinA =    P3_25;
constexpr Pin EncoderPinB =    P3_26;
constexpr Pin EncoderPinSw =   P1_30; //click
constexpr Pin PanelButtonPin = P2_11; //Extra button on Viki and RRD Panels (reset/back etc configurable)



#endif
