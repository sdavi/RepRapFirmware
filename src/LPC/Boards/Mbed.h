#ifndef MBED_H
#define MBED_H

#include "../PINS_LPC.h"


// List of assignable pins and their mapping from names to MPU ports. This is indexed by logical pin number.
// The names must match user input that has been concerted to lowercase and had _ and - characters stripped out.
// Aliases are separate by the , character.
// If a pin name is prefixed by ! then this means the pin is hardware inverted. The same pin may have names for both the inverted and non-inverted cases,
// for example the inverted heater pins on the expansion connector are available as non-inverted servo pins on a DFueX.

//Mostly just the smoothie config - used for debugging on MBed Dev Board

constexpr PinEntry PinTable_Mbed[] =
{

    //Thermistors
    {P0_23, PinCapability::ainrw, "t1"},
    {P0_24, PinCapability::ainrw, "t2"},
    {P0_25, PinCapability::ainrw, "t3"},
    //{P0_26, PinCapability::ainrw, "t4"},

    //Endstops
    {P1_24, PinCapability::rwpwm, "xmin"},
    {P1_25, PinCapability::rwpwm, "xmax"},
    {P1_26, PinCapability::rwpwm, "ymin"},
    {P1_27, PinCapability::rwpwm, "ymax"},
    {P1_28, PinCapability::rwpwm, "zmin"},
    {P1_29, PinCapability::rwpwm, "zmax"},

    //Heaters and Fans (Big and Small Mosfets}
    {P1_23, PinCapability::rwpwm, "q5"  }, //(Big Mosfet)
    {P2_5,  PinCapability::rwpwm, "q6" },  //(Big Mosfet)
    {P2_7,  PinCapability::rwpwm, "q7" },  //(Big Mosfet)
    {P1_22, PinCapability::rwpwm, "q4" },  //(Small Mosfet)
    {P2_4,  PinCapability::rwpwm, "q8" },  //(Small Mosfet)
    {P2_6,  PinCapability::rwpwm, "q9" },  //(Small Mosfet)

    //Spare pins (also as LEDs)
    {P1_18, PinCapability::rwpwm, "led1"},
    {P1_20, PinCapability::rwpwm, "led2"},
    {P1_21, PinCapability::rwpwm, "led3"},
    {P1_23, PinCapability::rwpwm, "led4"}, // mbed led4 also (heater q5) on Smoothie config
    

    //Spare pins (or used for LCD)
    {P0_4, PinCapability::rwpwm,  "p0.4"},
    {P0_5, PinCapability::rwpwm,  "p0.5"},
    {P0_10, PinCapability::rwpwm, "p0.10"},
    {P2_0, PinCapability::rwpwm,  "p2.0"},
    {P2_1, PinCapability::rwpwm,  "p2.1"},

    {P0_26, PinCapability::rwpwm, "p0.26"},
    {P1_30, PinCapability::rwpwm, "p1.30"},
    {P1_31, PinCapability::rwpwm, "p1.31"},
};


constexpr BoardDefaults mbedDefaults =
{
    {P3_25,  P0_10, P0_19, P0_21,  P4_29},   //enablePins
    {P2_11,  P2_12, P2_10, P2_3,   P2_8},    //stepPins
    {P3_26,  P4_28, P0_20, P0_22,  P2_13},   //dirPins
    0,                                       //digiPot Factor
};

#endif




