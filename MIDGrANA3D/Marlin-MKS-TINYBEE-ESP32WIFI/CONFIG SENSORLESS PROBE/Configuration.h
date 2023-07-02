#define SERIAL_PORT -1
#define SERIAL_PORT_2 0
#define BAUDRATE 230400 // Standard baud rate, used because my Orange Pi Zero is incompatible with 250,000
#define MOTHERBOARD BOARD_BTT_SKR_V1_4 // File defining the motherboard pinouts
#define CUSTOM_MACHINE_NAME "CR-10S" // Nice name for the LCD screen
#define DEFAULT_NOMINAL_FILAMENT_DIA 1.75 // The Hemera is a 1.75 mm extruder
#define TEMP_SENSOR_0 5 // E3D's recommended configuration for their supplied thermistor
#define TEMP_SENSOR_BED 1 // Don't forget this or your heated bed won't be enabled
#define HEATER_0_MAXTEMP 300 // Slightly increased to allow going to 285C via the LCD
#define DEFAULT_Kp 35.49 // PID tuning for my Hemera, substitute your own values
#define DEFAULT_Ki 3.46 // PID tuning for my Hemera, substitute your own values
#define DEFAULT_Kd 91.07 // PID tuning for my Hemera, substitute your own values
#define DEFAULT_bedKp 222.11 // PID tuning for my CR-10S, substitute your own values
#define DEFAULT_bedKi 44.38 // PID tuning for my CR-10S, substitute your own values
#define DEFAULT_bedKd 741.11 // PID tuning for my CR-10S, substitute your own values
#define X_DRIVER_TYPE TMC2209 // I'm using a TMC2209 driver
#define Y_DRIVER_TYPE TMC2209 // I'm using a TMC2209 driver
#define Z_DRIVER_TYPE TMC2209 // I'm using a TMC2209 driver
#define E0_DRIVER_TYPE TMC2209 // I'm using a TMC2209 driver
#define DEFAULT_AXIS_STEPS_PER_UNIT { 80, 80, 400, 400 } // Defaults to be fine tuned (Hemera E0 steps are much higher than standard extruders)
#define DEFAULT_MAX_FEEDRATE { 300, 300, 20, 25 } // Increased Z-axis feed rate to speed up homing
//#define Z_MIN_PROBE_USES_Z_MIN_ENDSTOP_PIN // Comment this out because we're not using the Z-stop pin for BLTouch
#define BLTOUCH // Uncomment this to enable the BLTouch configuration
#define NOZZLE_TO_PROBE_OFFSET { -40, 0, -1.95 } // My probe is 40 mm 'left' and 2 mm 'below' nozzle when deployed
#define MIN_PROBE_EDGE 10 // Do not probe within 10 mm of edge
#define XY_PROBE_SPEED 10000 // I'm impatient, move quickly between probes
#define Z_PROBE_SPEED_FAST HOMING_FEEDRATE_Z // Z probing at full speed
#define Z_PROBE_SPEED_SLOW (Z_PROBE_SPEED_FAST / 2) // Accurate probe at half speed (previous versions was 1/8)
#define Z_CLEARANCE_DEPLOY_PROBE 5 // Don't retract full distance between probes
#define Z_CLEARANCE_BETWEEN_PROBES 5 // 5 mm is safe enough for clips, etc.
#define INVERT_X_DIR true // I found my X carriage going the opposite way.
#define INVERT_E0_DIR true // The Hemera would retract rather than extrude
#define X_BED_SIZE 300 // CR-10S bed width
#define Y_BED_SIZE 300 // CR-10S bed depth
#define X_MIN_POS -6 // StallGuard triggers the nozzle moves 6 mm beyond the print area to the left
#define Y_MIN_POS -34 // StallGuard triggers when the bed moves 34 mm beyond the print area to the back
#define X_MAX_POS 330 // The nozzle can go 30mm beyond the print area to the right
#define Z_MAX_POS 400 // The height of the print area
//#define MIN_SOFTWARE_ENDSTOPS // Disabled to allow the nozzle to move outside the defined minimum and maximums
//#define MAX_SOFTWARE_ENDSTOPS // Disabled to allow the nozzle to move outside the defined minimum and maximums
#define AUTO_BED_LEVELING_UBL // We're using UBL as it's a more advanced levelling strategy
#define RESTORE_LEVELING_AFTER_G28 // Ensure that homing the printer doesn't disable mesh levelling
#define G26_MESH_VALIDATION // Enable the G26 for printing an auto-generated test pattern
#define MESH_TEST_HOTEND_TEMP 200 // I set this to 200C as this is the most common for my material
#define MESH_INSET 10 // This is the area around the edge of the print area which cannot be probed
#define GRID_MAX_POINTS_X 10 // This probes 10 points along the width, for 100 points total (assuming Y is also 10)
#define LCD_BED_LEVELING // Enable the LCD menu to level the bed
#define LEVEL_BED_CORNERS // Enable the LCD menu item to level the corners
#define Z_SAFE_HOMING // This moves the probe to the centre of the bed for Z homing when using default settings
#define HOMING_FEEDRATE_Z  (20*60) // I'm impatient, so I put this at 20 mm/s
#define EEPROM_SETTINGS // This needed to save settings to EEPROM, for example Z probe offset and the mesh
#define PREHEAT_1_TEMP_HOTEND // Change default PLA nozzle preheating to 200C - based on my material
#define PREHEAT_1_TEMP_BED // Change default PLA bed preheating to 50C - based on my material
#define ENCODER_STEPS_PER_MENU_ITEM 1 // I found my CR-10S wheel to be too sensitive, so I decreased the encoder steps
#define INDIVIDUAL_AXIS_HOMING_MENU // This is very useful for fine-tuning bump sensitivity
#define SPEAKER // Printer go beep boop
#define CR10_STOCKDISPLAY // I'm using the CR-10S' default display
