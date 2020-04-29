LPC Port of RepRapFirmware
==========================

This is an experimental port of [dc42's RepRapFirmware](https://github.com/dc42/RepRapFirmware/) for LPC1768/LPC1769 based boards.  

*Note: This firmware does not show up as a mass storage device when connected to a computer. Physical access to the internal sdcard may be required in order to revert back or update.*

### Main Differences to [dc42's RepRapFirmware](https://github.com/dc42/RepRapFirmware)
The CPUs targeted in this port only have 64K RAM which is less than those that run dc42s RepRapFirmware. Further, there is also some differences between the CPUs, and the following outlines the main differences in this port:  
* A maximum of 4 files can be open at a time.
* Reduced write buffers for SDCard to save memory.
* External interrupts (i.e., fan rpm etc) are limited to 3.
* Reduced number of networking buffers and reduced MTU to save memory.
* Only 2 HTTP Sockets and Responders. Only 1 HTTP session at a time.
* Disabled Ftp and Telnet interfaces
* Configuration:
  * GCode [M350](https://duet3d.dozuki.com/Wiki/Gcode#Section_M350_Set_microstepping_mode) - Microstepping for boards included in this port is done via hardware and thus M350 is not required. You may include it in your config.g if you like, but the command has no effect on the operation of the firmware.
  * Some drivers (such as the DRV8825) require specifying the timing information as they require longer pulse timings than the configured default that can result in missed steps. Timing information for stepper drivers can be added using [M569](https://duet3d.dozuki.com/Wiki/Gcode#Section_M569_Set_motor_driver_direction_enable_polarity_and_step_pulse_timing). Timing information can usually be found in the stepper driver data sheets.    
  * Auto-calibration restrictions to save memory:
    * Maximum number of probe points of 121; and
    * Delta maximum calibration points of 16
  * To support the number of different boards, a /sys/board.txt config file on the SDCard is used to configure the hardware pin assignments. Some example board config files [can be found here](LPC/ExampleBoardConfig)
    * M122 P200 command is used to print the mappings have been loaded by board.txt  and displays all options supported by board.txt

**The LPC port is experimental and is likely to contain bugs - Use at your own risk**



Licence
=======
The source files in this project are licensed under GPLv3, see http://www.gnu.org/licenses/gpl-3.0.en.html. 
