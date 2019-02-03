#ifndef PINS_DUET_H__
#define PINS_DUET_H__

#include <boost/preprocessor/repetition/repeat.hpp>

#include "Microstepping.h"


const size_t NumFirmwareUpdateModules = 0;

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define FIRMWARE_NAME "RepRapFirmware for LPC176[8/9] based Boards"


#define SUPPORT_OBJECT_MODEL         0
// Features definition
#define HAS_CPU_TEMP_SENSOR		     0				// enabling the CPU temperature sensor disables Due pin 13 due to bug in SAM3X
#define HAS_HIGH_SPEED_SD		     0

#define HAS_VOLTAGE_MONITOR		     0
#define ACTIVE_LOW_HEAT_ON		     0

#define HAS_RTOSPLUSTCP_NETWORKING   1
#define HAS_LWIP_NETWORKING          0
#define HAS_WIFI_NETWORKING          0

#define HAS_VREF_MONITOR             0
#define SUPPORT_NONLINEAR_EXTRUSION  0

#define SUPPORT_INKJET		0					// set nonzero to support inkjet control
#define SUPPORT_ROLAND		0					// set nonzero to support Roland mill
#define SUPPORT_SCANNER		0					// set nonzero to support FreeLSS scanners
#define SUPPORT_IOBITS		0					// set to support P parameter in G0/G1 commands
#define SUPPORT_DHT_SENSOR	0					// set nonzero to support DHT temperature/humidity sensors

#define ENABLE_TELNET 0
#define ENABLE_FTP    0
#define NO_PANELDUE   1

#define SUPPORT_12864_LCD       1

constexpr size_t NumExtraHeaterProtections = 4;
constexpr size_t NumTachos = 0;
constexpr Pin TachoPins[NumTachos] = {  };

// The physical capabilities of the machine

#if defined(__AZTEEGX5MINI__)
# include "variants/AzteegX5Mini1_1.h"
#elif defined(__SMOOTHIEBOARD__)
# include "variants/Smoothieboard1.h"
#elif defined(__REARM__)
# include "variants/ReArm1_0.h"
#elif defined(__AZSMZ__)
# include "variants/AZSMZ.h"
#elif defined(__MKSSBASE__)
# include "variants/MksSbase.h"
#elif defined(__BIQUSKR__)
# include "variants/BIQUSKR.h"
#elif defined(__MBED__)
//Only used for debugging
# include "variants/MBed.h"
#else
# error "Unknown LPC Variant"
#endif

constexpr size_t NUM_SERIAL_CHANNELS = 2;
// Use TX0/RX0 for the auxiliary serial line

#if defined(__MBED__)
#define SERIAL_MAIN_DEVICE Serial0 //TX0/RX0 connected to via seperate USB
#define SERIAL_AUX_DEVICE Serial //USB pins unconnected.
#else
#define SERIAL_MAIN_DEVICE Serial //USB
#define SERIAL_AUX_DEVICE Serial0 //TX0/RX0

#endif



// This next definition defines the highest one.
const int HighestLogicalPin = 60 + ARRAY_SIZE(SpecialPinMap) - 1;		// highest logical pin number on this electronics

// Timer allocation
//SD: LPC1768 has 4 timers + 1 RIT (Repetitive Interrupt Timer)
//PCLK_timer1-4 is set to 1/4 CCLK by CORE...

//Timer 0 is used for Step Generation
#define STEP_TC				(LPC_TIM0)
#define STEP_TC_IRQN		TIMER0_IRQn
#define STEP_TC_HANDLER		TIMER0_IRQHandler
#define STEP_TC_PCONPBIT    SBIT_PCTIM0
#define STEP_TC_PCLKBIT     PCLK_TIMER0
#define STEP_TC_TIMER       TIMER0


//Timers 1-3 used for PWM Generation
//Each timer has 4 Match Registers. We will use 1 to generate the interrupt on the period and reset the counter
//Leaving 3 for PWM on each timer.
//We will use the following to create a full mapping of all pins, and indicating which pins are configured to use timerPWM
//and stored in flash. We do this so its a quick lookup rather than searching through arrays etc each time

//Uses Boost Repeat to create the 5 ports of 32 pins array

//Each entry will indicate which timer and which slot (of 3) the pin is
// The high 3 bits will indicate the slow and the lower 4 bits will be the associated timer to use.


//Lower 4 bits Indicate Timer
#define TimerSelection(n)   (uint8_t)( n == Timer1PWMPins[0] || n == Timer1PWMPins[1] || n == Timer1PWMPins[2] )?TimerPWM_1:\
                                     ( n == Timer2PWMPins[0] || n == Timer2PWMPins[1] || n == Timer2PWMPins[2] )?TimerPWM_2:\
                                     ( n == Timer3PWMPins[0] || n == Timer3PWMPins[1] || n == Timer3PWMPins[2] )?TimerPWM_3: 0
//Higher 4 bits indicate slot
#define SlotSelection(n)    (uint8_t)( n == Timer1PWMPins[0] || n == Timer2PWMPins[0] || n == Timer3PWMPins[0] )?TimerPWM_Slot1:\
                                     ( n == Timer1PWMPins[1] || n == Timer2PWMPins[1] || n == Timer3PWMPins[1] )?TimerPWM_Slot2:\
                                     ( n == Timer1PWMPins[2] || n == Timer2PWMPins[2] || n == Timer3PWMPins[2] )?TimerPWM_Slot3: 0


#define INITS(z, n, t) (uint8_t)( (TimerSelection(n)) | (SlotSelection(n)) ),
#define REP(n) BOOST_PP_REPEAT(n, INITS, item)

extern const uint8_t TimerPWMPinsArray[MaxPinNumber];


//EXternal Inetrrupt Pins
//To save memory we only allow 3, and we create a mapping for the ports similar to the TimerPWM above for fast lookups instead of searching arrays


#define ExtSlotSelection(n,t)  ( t == (ExternalInterruptPins[0]>>5)  && n == (ExternalInterruptPins[0] & 0x1f) )?ExtInt_Slot1:\
                                  ( t == (ExternalInterruptPins[1]>>5)  && n == (ExternalInterruptPins[1] & 0x1f) )?ExtInt_Slot2:\
                                  ( t == (ExternalInterruptPins[2]>>5)  && n == (ExternalInterruptPins[2] & 0x1f) )?ExtInt_Slot3: 0
//check the port and pin in the array and assign its slot number
#define EXTINITS(z, n, t) (uint8_t)(ExtSlotSelection(n,t)) ,

#define EXTINTREP(n, port) BOOST_PP_REPEAT(n, EXTINITS, port)



#endif
