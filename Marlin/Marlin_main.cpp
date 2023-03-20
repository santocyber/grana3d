/* -*- c++ -*- */

/*
    Reprap firmware based on Sprinter and grbl.
 Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 This firmware is a mashup between Sprinter and grbl.
  (https://github.com/kliment/Sprinter)
  (https://github.com/simen/grbl/tree)

 It has preliminary support for Matthew Roberts advance algorithm
    http://reprap.org/pipermail/reprap-dev/2011-May/003323.html
 */

#include "Marlin.h"

#ifdef ENABLE_AUTO_BED_LEVELING
#include "vector_3.h"
  #ifdef AUTO_BED_LEVELING_GRID
    #include "qr_solve.h"
  #endif
#endif // ENABLE_AUTO_BED_LEVELING

#include "Configuration.h"
#include "planner.h"
#include "stepper.h"
#include "temperature.h"
#include "motion_control.h"
#include "watchdog.h"
#include "ConfigurationStore.h"
#include "language.h"
#include "pins_arduino.h"
#include "math.h"

#define VERSION_STRING  "1.0.2"

extern char msgscreen_1[20],msgscreen_2[20],msgscreen_3[20];

/**
 * Look here for descriptions of G-codes:
 *  - http://linuxcnc.org/handbook/gcode/g-code.html
 *  - http://objects.reprap.org/wiki/Mendel_User_Manual:_RepRapGCodes
 *
 * Help us document these G-codes online:
 *  - http://marlinfirmware.org/index.php/G-Code
 *  - http://reprap.org/wiki/G-code
 *
 * -----------------
 * Implemented Codes
 * -----------------
 *
 * "G" Codes
 *
 * G0  -> G1
 * G1  - Coordinated Movement X Y Z E
 * G2  - CW ARC
 * G3  - CCW ARC
 * G4  - Dwell S<seconds> or P<milliseconds>
 * G28 - Home one or more axes
 * G30 - Single Z probe, probes bed at current XY location.
 * G31 - Dock sled (Z_PROBE_SLED only)
 * G32 - Undock sled (Z_PROBE_SLED only)
 * G90 - Use Absolute Coordinates
 * G91 - Use Relative Coordinates
 * G92 - Set current position to coordinates given
 *
 * "M" Codes
 *
 * M17  - Enable/Power all stepper motors
 * M18  - Disable all stepper motors; same as M84
 * M31  - Output time since last M109 or SD card start to serial
 * M42  - Change pin status via gcode Use M42 Px Sy to set pin x to value y, when omitting Px the onboard led will be used.
 * M48  - Measure Z_Probe repeatability. M48 [P # of points] [X position] [Y position] [V_erboseness #] [E_ngage Probe] [L # of legs of travel]
 * M80  - Turn on Power Supply
 * M81  - Turn off Power Supply
 * M82  - Set E codes absolute (default)
 * M83  - Set E codes relative while in Absolute Coordinates (G90) mode
 * M84  - Disable steppers until next move,
 *        or use S<seconds> to specify an inactivity timeout, after which the steppers will be disabled.  S0 to disable the timeout.
 * M85  - Set inactivity shutdown timer with parameter S<seconds>. To disable set zero (default)
 * M92  - Set axis_steps_per_unit - same syntax as G92
 * M104 - Set extruder target temp
 * M105 - Read current temp
 * M106 - Fan on
 * M107 - Fan off
 * M109 - Sxxx Wait for extruder current temp to reach target temp. Waits only when heating
 *        Rxxx Wait for extruder current temp to reach target temp. Waits when heating and cooling
 *        IF AUTOTEMP is enabled, S<mintemp> B<maxtemp> F<factor>. Exit autotemp by any M109 without F
 * M112 - Emergency stop
 * M114 - Output current position to serial port
 * M115 - Capabilities string
 * M119 - Output Endstop status to serial port
 * M120 - Enable endstop detection
 * M121 - Disable endstop detection
 * M140 - Set bed target temp
 * M190 - Sxxx Wait for bed current temp to reach target temp. Waits only when heating
 *        Rxxx Wait for bed current temp to reach target temp. Waits when heating and cooling
 * M200 - set filament diameter and set E axis units to cubic millimeters (use S0 to set back to millimeters).:D<millimeters>-
 * M201 - Set max acceleration in units/s^2 for print moves (M201 X1000 Y1000)
 * M202 - Set max acceleration in units/s^2 for travel moves (M202 X1000 Y1000) Unused in Marlin!!
 * M203 - Set maximum feedrate that your machine can sustain (M203 X200 Y200 Z300 E10000) in mm/sec
 * M204 - Set default acceleration: P for Printing moves, R for Retract only (no X, Y, Z) moves and T for Travel (non printing) moves (ex. M204 P800 T3000 R9000) in mm/sec^2
 * M205 -  advanced settings:  minimum travel speed S=while printing T=travel only,  B=minimum segment time X= maximum xy jerk, Z=maximum Z jerk, E=maximum E jerk
 * M206 - Set additional homing offset
 * M218 - Set hotend offset (in mm): T<extruder_number> X<offset_on_X> Y<offset_on_Y>
 * M220 - Set speed factor override percentage: S<factor in percent>
 * M221 - Set extrude factor override percentage: S<factor in percent>
 * M226 - Wait until the specified pin reaches the state required: P<pin number> S<pin state>
 * M300 - Play beep sound S<frequency Hz> P<duration ms>
 * M301 - Set PID parameters P I and D
 * M302 - Allow cold extrudes, or set the minimum extrude S<temperature>.
 * M303 - PID relay autotune S<temperature> sets the target temperature. (default target temperature = 150C)
 * M304 - Set bed PID parameters P I and D
 * M400 - Finish all moves
 * M401 - Lower Z probe if present
 * M402 - Raise Z probe if present
 * M410 - Quickstop. Abort all the planned moves
 * M420 - Enable/Disable Mesh Leveling (with current values) S1=enable S0=disable
 * M421 - Set a single Z coordinate in the Mesh Leveling grid. X<mm> Y<mm> Z<mm>
 * M428 - Set the home_offset logically based on the current_position
 * M500 - Store parameters in EEPROM
 * M501 - Read parameters from EEPROM (if you need reset them after you changed them temporarily).
 * M502 - Revert to the default "factory settings". You still need to store them in EEPROM afterwards if you want to.
 * M503 - Print the current settings (from memory not from EEPROM). Use S0 to leave off headings.
 * M600 - Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
 *
 *
 * ************ Custom codes - This can change to suit future G-code regulations
 * M100 - Watch Free Memory (For Debugging Only)
 * M851 - Set Z probe's Z offset (mm above extruder -- The value will always be negative)
 * M928 - Start SD logging (M928 filename.g) - ended by M29
 * M999 - Restart after being stopped by error
 *
 * "T" Codes
 *
 * T0-T3 - Select a tool by index (usually an extruder) [ F<mm/min> ]
 *
 */
 
//Stepper Movement Variables

//===========================================================================
//=============================public variables=============================
//===========================================================================

float homing_feedrate[] = HOMING_FEEDRATE;
bool axis_relative_modes[] = AXIS_RELATIVE_MODES;
int feedmultiply=100; //100->1 200->2
int saved_feedmultiply;
int extrudemultiply=100; //100->1 200->2
int extruder_multiply[EXTRUDERS] = {100
  #if EXTRUDERS > 1
    , 100
    #if EXTRUDERS > 2
      , 100
    #endif
  #endif
};
bool volumetric_enabled = false;
float filament_size[EXTRUDERS] = ARRAY_BY_EXTRUDERS1(DEFAULT_NOMINAL_FILAMENT_DIA);
float volumetric_multiplier[EXTRUDERS] = ARRAY_BY_EXTRUDERS1(1.0);
float current_position[NUM_AXIS] = { 0.0, 0.0, 0.0, 0.0 };
float add_homing[3]={0,0,0};

#define PROBE_STATE_EXTENDED 1
#define PROBE_STATE_RETRACTED 0
#define PROBE_STATE_UNKNOWN -1
int z_probe_state = PROBE_STATE_UNKNOWN;

float min_pos[3] = { X_MIN_POS, Y_MIN_POS, Z_MIN_POS };
float max_pos[3] = { X_MAX_POS, Y_MAX_POS, Z_MAX_POS };
bool axis_known_position[3] = {false, false, false};
float zprobe_zoffset;

// Extruder offset
#if EXTRUDERS > 1
#define NUM_EXTRUDER_OFFSETS 3 // supports offsets in XYZ plane
float extruder_offset[NUM_EXTRUDER_OFFSETS][EXTRUDERS] = {
#if defined(EXTRUDER_OFFSET_X) && defined(EXTRUDER_OFFSET_Y)
  EXTRUDER_OFFSET_X, EXTRUDER_OFFSET_Y
#endif
};
#endif
uint8_t active_extruder = 0;
int fanSpeed=0;

bool cancel_heatup = false ;

const char errormagic[] PROGMEM = "Error:";
const char echomagic[] PROGMEM = "echo:";

//===========================================================================
//=============================Private Variables=============================
//===========================================================================
const char axis_codes[NUM_AXIS] = {'X', 'Y', 'Z', 'E'};
static float destination[NUM_AXIS] = {  0.0, 0.0, 0.0, 0.0};

#ifndef DELTA
static float delta[3] = {0.0, 0.0, 0.0};
#endif

static float offset[3] = {0.0, 0.0, 0.0};
static bool home_all_axis = true;
static float feedrate = 1500.0, next_feedrate, saved_feedrate;
static long gcode_N, gcode_LastN, Stopped_gcode_LastN = 0;

static bool relative_mode = false;  //Determines Absolute or Relative Coordinates

static char cmdbuffer[BUFSIZE][MAX_CMD_SIZE];
static bool fromsd[BUFSIZE];
static int bufindr = 0;
static int bufindw = 0;
static int buflen = 0;
//static int i = 0;
static char serial_char;
static int serial_count = 0;
static boolean comment_mode = false;
static char *strchr_pointer; // just a pointer to find chars in the command string like X, Y, Z, E, etc

const int sensitive_pins[] = SENSITIVE_PINS; // Sensitive pin list for M42

//static float tt = 0;
//static float bt = 0;

//Inactivity shutdown variables
static unsigned long previous_millis_cmd = 0;
static unsigned long max_inactive_time = 0;
static unsigned long stepper_inactive_time = DEFAULT_STEPPER_DEACTIVE_TIME*1000l;

unsigned long starttime=0;
unsigned long stoptime=0;

static uint8_t tmp_extruder;


bool Stopped=false;

bool CooldownNoWait = true;
bool target_direction;

//===========================================================================
//=============================Routines======================================
//===========================================================================

void get_arc_coordinates();
bool setTargetedHotend(int code);

void serial_echopair_P(const char *s_P, float v)
    { serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char *s_P, double v)
    { serialprintPGM(s_P); SERIAL_ECHO(v); }
void serial_echopair_P(const char *s_P, unsigned long v)
    { serialprintPGM(s_P); SERIAL_ECHO(v); }

  extern "C" {
    extern unsigned int __bss_end;
    extern unsigned int __heap_start;
    extern void *__brkval;

    int freeMemory() {
      int free_memory;

      if ((int)__brkval == 0)
        free_memory = ((int)&free_memory) - ((int)&__bss_end);
      else
        free_memory = ((int)&free_memory) - ((int)__brkval);

      return free_memory;
    }
  }


void lo_beep()
{
          tone(BEEPER, 1000);
          delay(100);
          noTone(BEEPER);
}

//adds an command to the main command buffer
//thats really done in a non-safe way.
//needs overworking someday
void enquecommand(const char *cmd)
{
  if(buflen < BUFSIZE)
  {
    //this is dangerous if a mixing of serial and this happens
    strcpy(&(cmdbuffer[bufindw][0]),cmd);
    SERIAL_ECHO_START;
    SERIAL_ECHOPGM(MSG_Enqueing);
    SERIAL_ECHO(cmdbuffer[bufindw]);
    SERIAL_ECHOLNPGM("\"");
    bufindw= (bufindw + 1)%BUFSIZE;
    buflen += 1;
  }
}

void enquecommand_P(const char *cmd)
{
  if(buflen < BUFSIZE)
  {
    //this is dangerous if a mixing of serial and this happens
    strcpy_P(&(cmdbuffer[bufindw][0]),cmd);
    SERIAL_ECHO_START;
    SERIAL_ECHOPGM(MSG_Enqueing);
    SERIAL_ECHO(cmdbuffer[bufindw]);
    SERIAL_ECHOLNPGM("\"");
    bufindw= (bufindw + 1)%BUFSIZE;
    buflen += 1;
  }
}

void setup_killpin()
{
  #if defined(KILL_PIN) && KILL_PIN > -1
    SET_INPUT(KILL_PIN);
    WRITE(KILL_PIN,HIGH);
  #endif
}

// Set home pin
void setup_homepin(void)
{
#if defined(HOME_PIN) && HOME_PIN > -1
   SET_INPUT(HOME_PIN);
   WRITE(HOME_PIN,HIGH);
#endif
}


void setup_photpin()
{
  #if defined(PHOTOGRAPH_PIN) && PHOTOGRAPH_PIN > -1
    SET_OUTPUT(PHOTOGRAPH_PIN);
    WRITE(PHOTOGRAPH_PIN, LOW);
  #endif
}

void setup_powerhold()
{
  #if defined(SUICIDE_PIN) && SUICIDE_PIN > -1
    SET_OUTPUT(SUICIDE_PIN);
    WRITE(SUICIDE_PIN, HIGH);
  #endif
  #if defined(PS_ON_PIN) && PS_ON_PIN > -1
    SET_OUTPUT(PS_ON_PIN);
	#if defined(PS_DEFAULT_OFF)
	  WRITE(PS_ON_PIN, PS_ON_ASLEEP);
    #else
	  WRITE(PS_ON_PIN, PS_ON_AWAKE);
	#endif
  #endif
}

inline void line_to_destination(float mm_m) {
  plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], mm_m / 60, active_extruder);
}
inline void line_to_destination() {
  line_to_destination(feedrate);
}
inline void sync_plan_position() {
  plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
}

void suicide()
{
  #if defined(SUICIDE_PIN) && SUICIDE_PIN > -1
    SET_OUTPUT(SUICIDE_PIN);
    WRITE(SUICIDE_PIN, LOW);
  #endif
}

void setup()
{
  setup_killpin();
  setup_powerhold();
  MYSERIAL.begin(BAUDRATE);
  SERIAL_PROTOCOLLNPGM("start");
  SERIAL_ECHO_START;

  // Check startup - does nothing if bootloader sets MCUSR to 0
  byte mcu = MCUSR;
  if(mcu & 1) SERIAL_ECHOLNPGM(MSG_POWERUP);
  if(mcu & 2) SERIAL_ECHOLNPGM(MSG_EXTERNAL_RESET);
  if(mcu & 4) SERIAL_ECHOLNPGM(MSG_BROWNOUT_RESET);
  if(mcu & 8) SERIAL_ECHOLNPGM(MSG_WATCHDOG_RESET);
  if(mcu & 32) SERIAL_ECHOLNPGM(MSG_SOFTWARE_RESET);
  MCUSR=0;
  SERIAL_PROTOCOLLNPGM("start");
  SERIAL_ECHOPGM(MSG_MARLIN);
  SERIAL_ECHOLNPGM(VERSION_STRING);
  #ifdef STRING_VERSION_CONFIG_H
    #ifdef STRING_CONFIG_H_AUTHOR
      SERIAL_ECHO_START;
      SERIAL_ECHOPGM(MSG_CONFIGURATION_VER);
      SERIAL_ECHOPGM(STRING_VERSION_CONFIG_H);
      SERIAL_ECHOPGM(MSG_AUTHOR);
      SERIAL_ECHOLNPGM(STRING_CONFIG_H_AUTHOR);
      SERIAL_ECHOPGM("Compiled: ");
      SERIAL_ECHOLNPGM(__DATE__);
    #endif
  #endif
  SERIAL_ECHO_START;
  SERIAL_ECHOPGM(MSG_FREE_MEMORY);
  SERIAL_ECHO(freeMemory());
  SERIAL_ECHOPGM(MSG_PLANNER_BUFFER_BYTES);
  SERIAL_ECHOLN((int)sizeof(block_t)*BLOCK_BUFFER_SIZE);
  for(int8_t i = 0; i < BUFSIZE; i++)
  {
    fromsd[i] = false;
  }

  // loads data from EEPROM if available else uses defaults (and resets step acceleration rate)
  Config_RetrieveSettings();

  tp_init();    // Initialize temperature loop
  plan_init();  // Initialize planner;
  watchdog_init();
  st_init();    // Initialize stepper, this enables interrupts!
  setup_photpin();

//removed lcd reference rcf
  _delay_ms(1000);	// wait 1sec to display the splash screen

  #if defined(CONTROLLERFAN_PIN) && CONTROLLERFAN_PIN > -1
    SET_OUTPUT(CONTROLLERFAN_PIN); //Set pin used for driver cooling fan
  #endif

  setup_homepin();
}


void loop()
{
  if(buflen < (BUFSIZE-1))
    get_command();
  if(buflen)
  {
    process_commands();
    buflen = (buflen-1);
    bufindr = (bufindr + 1)%BUFSIZE;
  }
  //check heater every n milliseconds
  manage_heater();
  manage_inactivity();
  checkHitEndstops();

}

void get_command()
{
  while( MYSERIAL.available() > 0  && buflen < BUFSIZE) {
    serial_char = MYSERIAL.read();
    if(serial_char == '\n' ||
       serial_char == '\r' ||
       (serial_char == ':' && comment_mode == false) ||
       serial_count >= (MAX_CMD_SIZE - 1) )
    {
      if(!serial_count) { //if empty line
        comment_mode = false; //for new command
        return;
      }
      cmdbuffer[bufindw][serial_count] = 0; //terminate string
      if(!comment_mode){
        comment_mode = false; //for new command
        fromsd[bufindw] = false;
        if(strchr(cmdbuffer[bufindw], 'N') != NULL)
        {
          strchr_pointer = strchr(cmdbuffer[bufindw], 'N');
          gcode_N = (strtol(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL, 10));
          if(gcode_N != gcode_LastN+1 && (strstr_P(cmdbuffer[bufindw], PSTR("M110")) == NULL) ) {
            SERIAL_ERROR_START;
            SERIAL_ERRORPGM(MSG_ERR_LINE_NO);
            SERIAL_ERRORLN(gcode_LastN);
            //Serial.println(gcode_N);
            FlushSerialRequestResend();
            serial_count = 0;
            return;
          }

          if(strchr(cmdbuffer[bufindw], '*') != NULL)
          {
            byte checksum = 0;
            byte count = 0;
            while(cmdbuffer[bufindw][count] != '*') checksum = checksum^cmdbuffer[bufindw][count++];
            strchr_pointer = strchr(cmdbuffer[bufindw], '*');

            if( (int)(strtod(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL)) != checksum) {
              SERIAL_ERROR_START;
              SERIAL_ERRORPGM(MSG_ERR_CHECKSUM_MISMATCH);
              SERIAL_ERRORLN(gcode_LastN);
              FlushSerialRequestResend();
              serial_count = 0;
              return;
            }
            //if no errors, continue parsing
          }
          else
          {
            SERIAL_ERROR_START;
            SERIAL_ERRORPGM(MSG_ERR_NO_CHECKSUM);
            SERIAL_ERRORLN(gcode_LastN);
            FlushSerialRequestResend();
            serial_count = 0;
            return;
          }

          gcode_LastN = gcode_N;
          //if no errors, continue parsing
        }
        else  // if we don't receive 'N' but still see '*'
        {
          if((strchr(cmdbuffer[bufindw], '*') != NULL))
          {
            SERIAL_ERROR_START;
            SERIAL_ERRORPGM(MSG_ERR_NO_LINENUMBER_WITH_CHECKSUM);
            SERIAL_ERRORLN(gcode_LastN);
            serial_count = 0;
            return;
          }
        }
        if((strchr(cmdbuffer[bufindw], 'G') != NULL)){
          strchr_pointer = strchr(cmdbuffer[bufindw], 'G');
          switch((int)((strtod(&cmdbuffer[bufindw][strchr_pointer - cmdbuffer[bufindw] + 1], NULL)))){
          case 0:
          case 1:
          case 2:
          case 3:
            if (Stopped == true) {
              SERIAL_ERRORLNPGM(MSG_ERR_STOPPED);

            }
            break;
          default:
            break;
          }

        }

        //If command was e-stop process now
        if(strcmp(cmdbuffer[bufindw], "M112") == 0)
          kill();
        
        bufindw = (bufindw + 1)%BUFSIZE;
        buflen += 1;
      }
      serial_count = 0; //clear buffer
    }
    else
    {
      if(serial_char == ';') comment_mode = true;
      if(!comment_mode) cmdbuffer[bufindw][serial_count++] = serial_char;
    }
  }
}


float code_value()
{
  return (strtod(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL));
}

long code_value_long()
{
  return (strtol(&cmdbuffer[bufindr][strchr_pointer - cmdbuffer[bufindr] + 1], NULL, 10));
}

bool code_seen(char code)
{
  strchr_pointer = strchr(cmdbuffer[bufindr], code);
  return (strchr_pointer != NULL);  //Return True if a character was found
}

#define DEFINE_PGM_READ_ANY(type, reader)       \
    static inline type pgm_read_any(const type *p)  \
    { return pgm_read_##reader##_near(p); }

DEFINE_PGM_READ_ANY(float,       float);
DEFINE_PGM_READ_ANY(signed char, byte);

#define XYZ_CONSTS_FROM_CONFIG(type, array, CONFIG) \
static const PROGMEM type array##_P[3] =        \
    { X_##CONFIG, Y_##CONFIG, Z_##CONFIG };     \
static inline type array(int axis)          \
    { return pgm_read_any(&array##_P[axis]); }

XYZ_CONSTS_FROM_CONFIG(float, base_min_pos,    MIN_POS);
XYZ_CONSTS_FROM_CONFIG(float, base_max_pos,    MAX_POS);
XYZ_CONSTS_FROM_CONFIG(float, base_home_pos,   HOME_POS);
XYZ_CONSTS_FROM_CONFIG(float, max_length,      MAX_LENGTH);
XYZ_CONSTS_FROM_CONFIG(float, home_retract_mm, HOME_RETRACT_MM);
XYZ_CONSTS_FROM_CONFIG(signed char, home_dir,  HOME_DIR);

static void axis_is_at_home(int axis) {
  current_position[axis] = base_home_pos(axis) + add_homing[axis];
  min_pos[axis] =          base_min_pos(axis) + add_homing[axis];
  max_pos[axis] =          base_max_pos(axis) + add_homing[axis];
}

#ifdef ENABLE_AUTO_BED_LEVELING
#ifdef AUTO_BED_LEVELING_GRID
static void set_bed_level_equation_lsq(double *plane_equation_coefficients)
{
    vector_3 planeNormal = vector_3(-plane_equation_coefficients[0], -plane_equation_coefficients[1], 1);
    planeNormal.debug((char*)"planeNormal");
    plan_bed_level_matrix = matrix_3x3::create_look_at(planeNormal);
    //bedLevel.debug("bedLevel");

    //plan_bed_level_matrix.debug("bed level before");
    //vector_3 uncorrected_position = plan_get_position_mm();
    //uncorrected_position.debug("position before");

    vector_3 corrected_position = plan_get_position();
//    corrected_position.debug("position after");
    current_position[X_AXIS] = corrected_position.x;
    current_position[Y_AXIS] = corrected_position.y;
    current_position[Z_AXIS] = corrected_position.z;

    // put the bed at 0 so we don't go below it.
    current_position[Z_AXIS] = zprobe_zoffset; // in the lsq we reach here after raising the extruder due to the loop structure

    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
}

#else // not AUTO_BED_LEVELING_GRID

static void set_bed_level_equation_3pts(float z_at_pt_1, float z_at_pt_2, float z_at_pt_3) {

    plan_bed_level_matrix.set_to_identity();

    vector_3 pt1 = vector_3(ABL_PROBE_PT_1_X, ABL_PROBE_PT_1_Y, z_at_pt_1);
    vector_3 pt2 = vector_3(ABL_PROBE_PT_2_X, ABL_PROBE_PT_2_Y, z_at_pt_2);
    vector_3 pt3 = vector_3(ABL_PROBE_PT_3_X, ABL_PROBE_PT_3_Y, z_at_pt_3);

    vector_3 from_2_to_1 = (pt1 - pt2).get_normal();
    vector_3 from_2_to_3 = (pt3 - pt2).get_normal();
    vector_3 planeNormal = vector_3::cross(from_2_to_1, from_2_to_3).get_normal();
    planeNormal = vector_3(planeNormal.x, planeNormal.y, abs(planeNormal.z));

    plan_bed_level_matrix = matrix_3x3::create_look_at(planeNormal);

    vector_3 corrected_position = plan_get_position();
    current_position[X_AXIS] = corrected_position.x;
    current_position[Y_AXIS] = corrected_position.y;
    current_position[Z_AXIS] = corrected_position.z;

    // put the bed at 0 so we don't go below it.
    current_position[Z_AXIS] = zprobe_zoffset;

    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

}

#endif // AUTO_BED_LEVELING_GRID

static void run_z_probe() 
{
    plan_bed_level_matrix.set_to_identity();
    feedrate = homing_feedrate[Z_AXIS];

    // move down until you find the bed
    float zPosition = -10;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

        // we have to let the planner know where we are right now as it is not where we said to go.
    zPosition = st_get_position_mm(Z_AXIS);
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS]);

    // move up the retract distance
    zPosition += home_retract_mm(Z_AXIS);
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    // move back down slowly to find bed
    feedrate = homing_feedrate[Z_AXIS]/4;
    zPosition -= home_retract_mm(Z_AXIS) * 2;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], zPosition, current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    current_position[Z_AXIS] = st_get_position_mm(Z_AXIS);
    // make sure the planner knows where we are as it may be a bit different than we last said to move to
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
}

static void do_blocking_move_to(float x, float y, float z) 
{
    float oldFeedRate = feedrate;

    feedrate = homing_feedrate[Z_AXIS];

    current_position[Z_AXIS] = z;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    feedrate = XY_TRAVEL_SPEED;

    current_position[X_AXIS] = x;
    current_position[Y_AXIS] = y;
    plan_buffer_line(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    feedrate = oldFeedRate;
}

static void do_blocking_move_relative(float offset_x, float offset_y, float offset_z) 
{
    do_blocking_move_to(current_position[X_AXIS] + offset_x, current_position[Y_AXIS] + offset_y, current_position[Z_AXIS] + offset_z);
}

static void setup_for_endstop_move() {
    saved_feedrate = feedrate;
    saved_feedmultiply = feedmultiply;
    feedmultiply = 100;
    previous_millis_cmd = millis();

    enable_endstops(true);
}

static void clean_up_after_endstop_move() {
#ifdef ENDSTOPS_ONLY_FOR_HOMING
    enable_endstops(false);
#endif

    feedrate = saved_feedrate;
    feedmultiply = saved_feedmultiply;
    previous_millis_cmd = millis();
}

static void engage_z_probe() 
{  
    if(z_probe_state != PROBE_STATE_EXTENDED) 
    {
      st_synchronize();
      // raise the z axis a little bit and then move x axis all the way to the right to raise the probe
      float x_before = current_position[X_AXIS]; 
      float z_before = current_position[Z_AXIS];
      if (z_before>=Z_RAISE_BEFORE_HOMING)
      {
        do_blocking_move_to(X_MIN_POS, current_position[Y_AXIS], current_position[Z_AXIS]);
      }
      else
      {
        do_blocking_move_to(X_MIN_POS, current_position[Y_AXIS], Z_RAISE_BEFORE_HOMING); 
      }
      z_probe_state = PROBE_STATE_EXTENDED;
      do_blocking_move_to(x_before, current_position[Y_AXIS], current_position[Z_AXIS]); // put X back where we found it
    }
}

static void retract_z_probe() 
{
    if(z_probe_state != PROBE_STATE_RETRACTED) 
    {
      st_synchronize();
      float x_before = current_position[X_AXIS]; 
      float z_before = current_position[Z_AXIS];

      // raise the z axis a little bit and then move x axis all the way to the right to raise the probe
      if (z_before>=Z_RAISE_BEFORE_HOMING)
      {
        do_blocking_move_to(X_MAX_POS, current_position[Y_AXIS], current_position[Z_AXIS]);
      }
      else
      {
        do_blocking_move_to(X_MAX_POS, current_position[Y_AXIS], Z_RAISE_BEFORE_HOMING); 
      }
      
      z_probe_state = PROBE_STATE_RETRACTED;

      // put X and Z back where we found them
      // if old X is near the X MIN, it will redeploy- so set to 0 if it's outside
      if (x_before<0)
      {
        x_before=0;
      }
      do_blocking_move_to(x_before, current_position[Y_AXIS], z_before);
    }
}

/// Probe bed height at position (x,y), returns the measured z value
static float probe_pt(float x, float y, float z_before,float z_after) 
{
  // move to right place
  do_blocking_move_to(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS] + z_before); // do the z move first

float xx = x - X_PROBE_OFFSET_FROM_EXTRUDER;
float yy = y - Y_PROBE_OFFSET_FROM_EXTRUDER;
  
  do_blocking_move_to(xx, yy, current_position[Z_AXIS]);

  #ifndef NO_RETRACT_BETWEEN_PROBINGS
  engage_z_probe();   
  #endif 

  run_z_probe();

  float measured_z = current_position[Z_AXIS];
  
  #ifndef NO_RETRACT_BETWEEN_PROBINGS
  retract_z_probe();
  #endif

  SERIAL_PROTOCOLPGM(MSG_BED);
  SERIAL_PROTOCOLPGM(" x: ");
  SERIAL_PROTOCOL(x);
  SERIAL_PROTOCOLPGM(" y: ");
  SERIAL_PROTOCOL(y);
  SERIAL_PROTOCOLPGM(" z: ");
  SERIAL_PROTOCOL(measured_z);
  SERIAL_PROTOCOLPGM("\n");
  return measured_z;
}

#endif // #ifdef ENABLE_AUTO_BED_LEVELING

#define HOMEAXIS_DO(LETTER) ((LETTER##_MIN_PIN > -1 && LETTER##_HOME_DIR==-1) || (LETTER##_MAX_PIN > -1 && LETTER##_HOME_DIR==1))

static void homeaxis(int axis) 
{
  if (axis==X_AXIS ? HOMEAXIS_DO(X) : axis==Y_AXIS ? HOMEAXIS_DO(Y) : axis==Z_AXIS ? HOMEAXIS_DO(Z) : 0) 
  {
    if (axis==X_AXIS)
    {
      SERIAL_PROTOCOLPGM("Homing X Axis\n");  
    }
    if (axis==Y_AXIS)
    {
      SERIAL_PROTOCOLPGM("Homing Y Axis\n");  
    }
    if (axis==Z_AXIS)
    {
      SERIAL_PROTOCOLPGM("Homing Z Axis\n");  
    }
    int axis_home_dir = home_dir(axis);
    if(axis==Z_AXIS && z_probe_state != PROBE_STATE_EXTENDED)
    {
      engage_z_probe();
    }

    current_position[axis] = 0;
    sync_plan_position();

    destination[axis] = 1.5 * max_length(axis) * axis_home_dir;
    feedrate = homing_feedrate[axis];
    line_to_destination();
    st_synchronize();

    current_position[axis] = 0;
    sync_plan_position(); //plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
    
    enable_endstops(false); // disable endstops while moving away
    
    destination[axis] = -home_retract_mm(axis) * axis_home_dir;
    line_to_destination(); //plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();

    enable_endstops(true);
    
    destination[axis] = 2*home_retract_mm(axis) * axis_home_dir;
   
    feedrate = homing_feedrate[axis]/2 ;

    line_to_destination(); //plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
    st_synchronize();
    
    axis_is_at_home(axis);
    sync_plan_position();
    
    destination[axis] = current_position[axis];
    feedrate = 0.0;
    endstops_hit_on_purpose();
    axis_known_position[axis] = true;

    if (axis==Z_AXIS) retract_z_probe();

  }
}

void refresh_cmd_timeout(void)
{
  previous_millis_cmd = millis();
}

void go_home()
{
    #ifdef ENABLE_AUTO_BED_LEVELING
    plan_bed_level_matrix.set_to_identity();  //Reset the plane ("erase" all leveling data)
    #endif 

    saved_feedrate      = feedrate;
    saved_feedmultiply  = feedmultiply;
    feedmultiply        = 100;
    previous_millis_cmd = millis();
    enable_endstops(true);
    for(int8_t i=0; i < NUM_AXIS; i++) destination[i] = current_position[i];
    feedrate = 0.0;

    home_all_axis = !((code_seen(axis_codes[X_AXIS])) || (code_seen(axis_codes[Y_AXIS])) || (code_seen(axis_codes[Z_AXIS])));
    
    // raise z axis. this movement is not done to deploy the probe, just to make sure we are high enough that it can be.
    float z_before = current_position[Z_AXIS];
    if (z_before<Z_RAISE_BEFORE_HOMING)
    {
      do_blocking_move_to(current_position[X_AXIS], current_position[Y_AXIS], Z_RAISE_BEFORE_HOMING);
    }
    
    destination[Z_AXIS] = current_position[Z_AXIS];

    if((home_all_axis) || (code_seen(axis_codes[X_AXIS])))
    {
         homeaxis(X_AXIS);
         // after x has hit home, the z probe is in the extended state
         z_probe_state = PROBE_STATE_EXTENDED;
    }

    if((home_all_axis) || (code_seen(axis_codes[Y_AXIS]))) 
    {
      homeaxis(Y_AXIS); 
    }

    // Z Safe mode activated.
    if((home_all_axis) || (code_seen(axis_codes[Z_AXIS])))
    {
      //ensure X/Y axis homed
      if (!axis_known_position[X_AXIS])
      {
        homeaxis(X_AXIS);
      }
      if (!axis_known_position[Y_AXIS])
      {
        homeaxis(Y_AXIS);
      }

        destination[X_AXIS] = round(Z_SAFE_HOMING_X_POINT - X_PROBE_OFFSET_FROM_EXTRUDER);
        destination[Y_AXIS] = round(Z_SAFE_HOMING_Y_POINT - Y_PROBE_OFFSET_FROM_EXTRUDER);
        destination[Z_AXIS] = 0;    // Set destination away from bed
        feedrate = XY_TRAVEL_SPEED / 60;
        current_position[Z_AXIS] = 0;

        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
        plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate, active_extruder);
        st_synchronize();
        current_position[X_AXIS] = destination[X_AXIS];
        current_position[Y_AXIS] = destination[Y_AXIS];

        homeaxis(Z_AXIS);
        current_position[Z_AXIS] += zprobe_zoffset;  //Add Z_Probe offset (the distance is negative) 
    }

    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

    #ifdef ENDSTOPS_ONLY_FOR_HOMING
        enable_endstops(false);
    #endif

    feedrate = saved_feedrate;
    feedmultiply = saved_feedmultiply;
    previous_millis_cmd = millis();
    endstops_hit_on_purpose();
      
    return;
}


#ifdef ENABLE_AUTO_BED_LEVELING
void z_probe_leveling() 
{
     float x_tmp, y_tmp, z_tmp, real_z;
     
     // you probably got here via a G29 command.

     #if Z_MIN_PIN == -1
     #error "You must have a Z_MIN endstop in order to enable Auto Bed Leveling feature:  Z_MIN_PIN must point to a valid hardware pin."
     #endif

     // Prevent user from running a G29 without first homing in X and Y
     if(!(axis_known_position[X_AXIS] && axis_known_position[Y_AXIS]))
     {
        go_home();
     }
     
     st_synchronize();

     #ifdef NO_RETRACT_BETWEEN_PROBINGS
     engage_z_probe(); // if we won't extend/retract during each probe point, then extend it now before we start.
     #endif
     do_blocking_move_to(LEFT_PROBE_BED_POSITION,FRONT_PROBE_BED_POSITION,current_position[Z_AXIS]);

     // make sure the bed_level_rotation_matrix is identity or the planner will get it incorectly     
     plan_bed_level_matrix.set_to_identity();
     
     vector_3 uncorrected_position = plan_get_position();
     current_position[X_AXIS] = uncorrected_position.x;
     current_position[Y_AXIS] = uncorrected_position.y;
     current_position[Z_AXIS] = uncorrected_position.z;
     plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
     
     setup_for_endstop_move();

     feedrate = homing_feedrate[Z_AXIS];

//     return;

     #ifdef AUTO_BED_LEVELING_GRID
       // probe at the points of a lattice grid
       int xGridSpacing = (RIGHT_PROBE_BED_POSITION - LEFT_PROBE_BED_POSITION) / (AUTO_BED_LEVELING_GRID_POINTS-1);
       int yGridSpacing = (BACK_PROBE_BED_POSITION - FRONT_PROBE_BED_POSITION) / (AUTO_BED_LEVELING_GRID_POINTS-1);

            // solve the plane equation ax + by + d = z
            // A is the matrix with rows [x y 1] for all the probed points
            // B is the vector of the Z positions
            // the normal vector to the plane is formed by the coefficients of the plane equation in the standard form, which is Vx*x+Vy*y+Vz*z+d = 0
            // so Vx = -a Vy = -b Vz = 1 (we want the vector facing towards positive Z

            // "A" matrix of the linear system of equations
            double eqnAMatrix[AUTO_BED_LEVELING_GRID_POINTS*AUTO_BED_LEVELING_GRID_POINTS*3];
            // "B" vector of Z points
            double eqnBVector[AUTO_BED_LEVELING_GRID_POINTS*AUTO_BED_LEVELING_GRID_POINTS];
            int probePointCounter = 0;
            bool zig = true;
            int yProbe,xProbe;
                  
            for (yProbe=FRONT_PROBE_BED_POSITION; yProbe <= BACK_PROBE_BED_POSITION; yProbe += yGridSpacing)
            {
              int xInc;
              if (zig)
              {
                xProbe = LEFT_PROBE_BED_POSITION;                
                xInc = xGridSpacing;
                zig = false;
              } else // zag
              {
                xProbe = RIGHT_PROBE_BED_POSITION;                
                xInc = -xGridSpacing;
                zig = true;
              }

              for (int xCount=0; xCount < AUTO_BED_LEVELING_GRID_POINTS; xCount++)
              {
                float z_before;
                if (probePointCounter == 0)
                {
                  // raise before probing
                  z_before = Z_RAISE_BEFORE_PROBING;
                } 
                else
                {
                  // raise extruder
                  z_before = Z_RAISE_BETWEEN_PROBINGS;
                }

                float measured_z = probe_pt(xProbe, yProbe, z_before,Z_RAISE_AFTER_PROBING);
//            return;    
                eqnBVector[probePointCounter] = measured_z;

                eqnAMatrix[probePointCounter + 0*AUTO_BED_LEVELING_GRID_POINTS*AUTO_BED_LEVELING_GRID_POINTS] = xProbe;
                eqnAMatrix[probePointCounter + 1*AUTO_BED_LEVELING_GRID_POINTS*AUTO_BED_LEVELING_GRID_POINTS] = yProbe;
                eqnAMatrix[probePointCounter + 2*AUTO_BED_LEVELING_GRID_POINTS*AUTO_BED_LEVELING_GRID_POINTS] = 1;
                probePointCounter++;
                xProbe += xInc;
              }
            }
            
            clean_up_after_endstop_move();

            // solve lsq problem
            double *plane_equation_coefficients = qr_solve(AUTO_BED_LEVELING_GRID_POINTS*AUTO_BED_LEVELING_GRID_POINTS, 3, eqnAMatrix, eqnBVector);

            SERIAL_PROTOCOLPGM("Eqn coefficients: a: ");
            SERIAL_PROTOCOL(plane_equation_coefficients[0]);
            SERIAL_PROTOCOLPGM(" b: ");
            SERIAL_PROTOCOL(plane_equation_coefficients[1]);
            SERIAL_PROTOCOLPGM(" d: ");
            SERIAL_PROTOCOLLN(plane_equation_coefficients[2]);
            set_bed_level_equation_lsq(plane_equation_coefficients);
            
            free(plane_equation_coefficients);

#else // AUTO_BED_LEVELING_GRID not defined
            #ifdef NO_RETRACT_BETWEEN_PROBINGS
            engage_z_probe(); // if we won't extend/retract during each probe point, then extend it now before we start.
            #endif
            // Probe at 3 arbitrary points
            // probe 1
            float z_at_pt_1 = probe_pt(ABL_PROBE_PT_1_X, ABL_PROBE_PT_1_Y, Z_RAISE_BEFORE_PROBING,Z_RAISE_AFTER_PROBING);
            // probe 2
            float z_at_pt_2 = probe_pt(ABL_PROBE_PT_2_X, ABL_PROBE_PT_2_Y, current_position[Z_AXIS] + Z_RAISE_BETWEEN_PROBINGS,Z_RAISE_AFTER_PROBING);
            // probe 3
            float z_at_pt_3 = probe_pt(ABL_PROBE_PT_3_X, ABL_PROBE_PT_3_Y, current_position[Z_AXIS] + Z_RAISE_BETWEEN_PROBINGS,Z_RAISE_AFTER_PROBING);

            clean_up_after_endstop_move();
            set_bed_level_equation_3pts(z_at_pt_1, z_at_pt_2, z_at_pt_3);
#endif // AUTO_BED_LEVELING_GRID

    st_synchronize();

    // The following code correct the Z height difference from z-probe position and hotend tip position.
    // The Z height on homing is measured by Z-Probe, but the probe is quite far from the hotend.
    // When the bed is uneven, this height must be corrected.
    real_z = float(st_get_position(Z_AXIS))/axis_steps_per_unit[Z_AXIS];  //get the real Z (since the auto bed leveling is already correcting the plane)
    x_tmp = current_position[X_AXIS] + X_PROBE_OFFSET_FROM_EXTRUDER;
    y_tmp = current_position[Y_AXIS] + Y_PROBE_OFFSET_FROM_EXTRUDER;
    z_tmp = current_position[Z_AXIS];

    apply_rotation_xyz(plan_bed_level_matrix, x_tmp, y_tmp, z_tmp);         //Apply the correction sending the probe offset

    current_position[Z_AXIS] = z_tmp - real_z + current_position[Z_AXIS];   //The difference is added to current position and sent to planner.

//    current_position[X_AXIS] = 15; //x_tmp;
    
    plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
    
    #ifdef NO_RETRACT_BETWEEN_PROBINGS
    // first raise the Z up a little so that the probe can move freely

          tone(BEEPER, 2000);
          delay(100);
          noTone(BEEPER);

    //do_blocking_move_to(current_position[X_AXIS],current_position[Y_AXIS],current_position[Z_AXIS]+Z_RAISE_AFTER_PROBING);            
    retract_z_probe(); // if we didn't retract after each probe point, then do so now that we are done.
    #endif

    return;
}
#endif

void process_commands()
{
  unsigned long codenum; //throw away variable
  char *starpos = NULL;
  if(code_seen('G'))
  {
    switch((int)code_value())
    {
    case 0: // G0 -> G1
    case 1: // G1
      if(Stopped == false) {
        get_coordinates(); // For X Y Z E F

        prepare_move();
        //ClearToSend();
      }
      break;

    case 2: // G2  - CW ARC
      if(Stopped == false) {
        get_arc_coordinates();
        prepare_arc_move(true);
      }
      break;
    case 3: // G3  - CCW ARC
      if(Stopped == false) {
        get_arc_coordinates();
        prepare_arc_move(false);
      }
      break;

    case 4: // G4 dwell

      codenum = 0;
      if(code_seen('P')) codenum = code_value(); // milliseconds to wait
      if(code_seen('S')) codenum = code_value() * 1000; // seconds to wait

      st_synchronize();
      codenum += millis();  // keep track of when we started waiting
      previous_millis_cmd = millis();
      while(millis() < codenum) {
        manage_heater();
        manage_inactivity();

      }
      break;


    case 28: //G28 Home all Axis one at a time
    go_home();
    break;  
        
    case 30: // G30 Single Z Probe
        {
            engage_z_probe(); // Engage Z endstop if available
            st_synchronize();
            // TODO: make sure the bed_level_rotation_matrix is identity or the planner will get set incorectly
            setup_for_endstop_move();

            feedrate = homing_feedrate[Z_AXIS];

            run_z_probe();
            SERIAL_PROTOCOLPGM(MSG_BED);
            SERIAL_PROTOCOLPGM(" X: ");
            SERIAL_PROTOCOL(current_position[X_AXIS]);
            SERIAL_PROTOCOLPGM(" Y: ");
            SERIAL_PROTOCOL(current_position[Y_AXIS]);
            SERIAL_PROTOCOLPGM(" Z: ");
            SERIAL_PROTOCOL(current_position[Z_AXIS]);
            SERIAL_PROTOCOLPGM("\n");

            clean_up_after_endstop_move();
            retract_z_probe(); // Retract Z endstop if available
        }
        break;

    case 90: // G90
      relative_mode = false;
      break;
    case 91: // G91
      relative_mode = true;
      break;
    case 92: // G92
      if(!code_seen(axis_codes[E_AXIS]))
        st_synchronize();
      for(int8_t i=0; i < NUM_AXIS; i++) {
        if(code_seen(axis_codes[i])) {
           if(i == E_AXIS) {
             current_position[i] = code_value();
             plan_set_e_position(current_position[E_AXIS]);
           }
           else {
		        current_position[i] = code_value()+add_homing[i];
            plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);
           }
        }
      }
      break;
    }
  }

  else if(code_seen('M'))
  {
    switch( (int)code_value() )
    {

    case 17:

        enable_x();
        enable_y();
        enable_z();
        enable_e0();
        enable_e1();
        enable_e2();
      break;


    case 31: //M31 take time since the start of the SD print or an M109 command
      {
      stoptime=millis();
      char time[30];
      unsigned long t=(stoptime-starttime)/1000;
      int sec,min;
      min=t/60;
      sec=t%60;
      sprintf_P(time, PSTR("%i min, %i sec"), min, sec);
      SERIAL_ECHO_START;
      SERIAL_ECHOLN(time);

      autotempShutdown();
      }
      break;
    case 42: //M42 -Change pin status via gcode
      if (code_seen('S'))
      {
        int pin_status = code_value();
        int pin_number = LED_PIN;
        if (code_seen('P') && pin_status >= 0 && pin_status <= 255)
          pin_number = code_value();
        for(int8_t i = 0; i < (int8_t)(sizeof(sensitive_pins)/sizeof(int)); i++)
        {
          if (sensitive_pins[i] == pin_number)
          {
            pin_number = -1;
            break;
          }
        }
      #if defined(FAN_PIN) && FAN_PIN > -1
        if (pin_number == FAN_PIN)
          fanSpeed = pin_status;
      #endif
        if (pin_number > -1)
        {
          pinMode(pin_number, OUTPUT);
          digitalWrite(pin_number, pin_status);
          analogWrite(pin_number, pin_status);
        }
      }
     break;

// M48 Z-Probe repeatability measurement function.
//
// Usage:   M48 <n #_samples> <X X_position_for_samples> <Y Y_position_for_samples> <V Verbose_Level> <Engage_probe_for_each_reading> <L legs_of_movement_prior_to_doing_probe>
//	
// This function assumes the bed has been homed.  Specificaly, that a G28 command
// as been issued prior to invoking the M48 Z-Probe repeatability measurement function.
// Any information generated by a prior G29 Bed leveling command will be lost and need to be
// regenerated.
//
// The number of samples will default to 10 if not specified.  You can use upper or lower case
// letters for any of the options EXCEPT n.  n must be in lower case because Marlin uses a capital
// N for its communication protocol and will get horribly confused if you send it a capital N.
//

#ifdef ENABLE_AUTO_BED_LEVELING
#ifdef Z_PROBE_REPEATABILITY_TEST 

    case 48: // M48 Z-Probe repeatability
        {
            #if Z_MIN_PIN == -1
            #error "You must have a Z_MIN endstop in order to enable calculation of Z-Probe repeatability."
            #endif

	double sum=0.0; 
	double mean=0.0; 
	double sigma=0.0;
	double sample_set[50];
	int verbose_level=1, n=0, j, n_samples = 10, n_legs=0, engage_probe_for_each_reading=0 ;
	double X_current, Y_current, Z_current;
	double X_probe_location, Y_probe_location, Z_start_location, ext_position;
	
	if (code_seen('V') || code_seen('v')) {
        	verbose_level = code_value();
		if (verbose_level<0 || verbose_level>4 ) {
			SERIAL_PROTOCOLPGM("?Verbose Level not plausable.\n");
			goto Sigma_Exit;
		}
	}

	if (verbose_level > 0)   {
		SERIAL_PROTOCOLPGM("M48 Z-Probe Repeatability test.   Version 2.00\n");
		SERIAL_PROTOCOLPGM("Full support at: http://3dprintboard.com/forum.php\n");
	}

	if (code_seen('P') || code_seen('p')) {
        	n_samples = code_value();
		if (n_samples<4 || n_samples>50 ) {
			SERIAL_PROTOCOLPGM("?Specified sample size not plausable.\n");
			goto Sigma_Exit;
		}
	}

	X_current = X_probe_location = st_get_position_mm(X_AXIS);
	Y_current = Y_probe_location = st_get_position_mm(Y_AXIS);
	Z_current = st_get_position_mm(Z_AXIS);
	Z_start_location = st_get_position_mm(Z_AXIS) + Z_RAISE_BEFORE_PROBING;
	ext_position	 = st_get_position_mm(E_AXIS);

	if (code_seen('E') || code_seen('e') ) 
		engage_probe_for_each_reading++;

	if (code_seen('X') || code_seen('x') ) {
        	X_probe_location = code_value() -  X_PROBE_OFFSET_FROM_EXTRUDER;
		if (X_probe_location<X_MIN_POS || X_probe_location>X_MAX_POS ) {
			SERIAL_PROTOCOLPGM("?Specified X position out of range.\n");
			goto Sigma_Exit;
		}
	}

	if (code_seen('Y') || code_seen('y') ) {
        	Y_probe_location = code_value() -  Y_PROBE_OFFSET_FROM_EXTRUDER;
		if (Y_probe_location<Y_MIN_POS || Y_probe_location>Y_MAX_POS ) {
			SERIAL_PROTOCOLPGM("?Specified Y position out of range.\n");
			goto Sigma_Exit;
		}
	}

	if (code_seen('L') || code_seen('l') ) {
        	n_legs = code_value();
		if ( n_legs==1 ) 
			n_legs = 2;
		if ( n_legs<0 || n_legs>15 ) {
			SERIAL_PROTOCOLPGM("?Specified number of legs in movement not plausable.\n");
			goto Sigma_Exit;
		}
	}

//
// Do all the preliminary setup work.   First raise the probe.
//

        st_synchronize();
        plan_bed_level_matrix.set_to_identity();
	plan_buffer_line( X_current, Y_current, Z_start_location,
			ext_position,
    			homing_feedrate[Z_AXIS]/60,
			active_extruder);
        st_synchronize();

//
// Now get everything to the specified probe point So we can safely do a probe to
// get us close to the bed.  If the Z-Axis is far from the bed, we don't want to 
// use that as a starting point for each probe.
//
	if (verbose_level > 2) 
		SERIAL_PROTOCOL("Positioning probe for the test.\n");

	plan_buffer_line( X_probe_location, Y_probe_location, Z_start_location,
			ext_position,
    			homing_feedrate[X_AXIS]/60,
			active_extruder);
        st_synchronize();

	current_position[X_AXIS] = X_current = st_get_position_mm(X_AXIS);
	current_position[Y_AXIS] = Y_current = st_get_position_mm(Y_AXIS);
	current_position[Z_AXIS] = Z_current = st_get_position_mm(Z_AXIS);
	current_position[E_AXIS] = ext_position = st_get_position_mm(E_AXIS);

// 
// OK, do the inital probe to get us close to the bed.
// Then retrace the right amount and use that in subsequent probes
//

        engage_z_probe();	

	setup_for_endstop_move();
	run_z_probe();

	current_position[Z_AXIS] = Z_current = st_get_position_mm(Z_AXIS);
	Z_start_location = st_get_position_mm(Z_AXIS) + Z_RAISE_BEFORE_PROBING;

	plan_buffer_line( X_probe_location, Y_probe_location, Z_start_location,
			ext_position,
    			homing_feedrate[X_AXIS]/60,
			active_extruder);
        st_synchronize();
	current_position[Z_AXIS] = Z_current = st_get_position_mm(Z_AXIS);

	if (engage_probe_for_each_reading)
        	retract_z_probe();

        for( n=0; n<n_samples; n++) {

		do_blocking_move_to( X_probe_location, Y_probe_location, Z_start_location); // Make sure we are at the probe location

		if ( n_legs)  {
		double radius=0.0, theta=0.0, x_sweep, y_sweep;
		int rotational_direction, l;

			rotational_direction = (unsigned long) millis() & 0x0001;			// clockwise or counter clockwise
			radius = (unsigned long) millis() % (long) (X_MAX_LENGTH/4); 			// limit how far out to go 
			theta = (float) ((unsigned long) millis() % (long) 360) / (360./(2*3.1415926));	// turn into radians

			for( l=0; l<n_legs-1; l++) {
				if (rotational_direction==1)
					theta += (float) ((unsigned long) millis() % (long) 20) / (360.0/(2*3.1415926)); // turn into radians
				else
					theta -= (float) ((unsigned long) millis() % (long) 20) / (360.0/(2*3.1415926)); // turn into radians

				radius += (float) ( ((long) ((unsigned long) millis() % (long) 10)) - 5);
				if ( radius<0.0 )
					radius = -radius;

				X_current = X_probe_location + cos(theta) * radius;
				Y_current = Y_probe_location + sin(theta) * radius;

				if ( X_current<X_MIN_POS)		// Make sure our X & Y are sane
					 X_current = X_MIN_POS;
				if ( X_current>X_MAX_POS)
					 X_current = X_MAX_POS;

				if ( Y_current<Y_MIN_POS)		// Make sure our X & Y are sane
					 Y_current = Y_MIN_POS;
				if ( Y_current>Y_MAX_POS)
					 Y_current = Y_MAX_POS;

				if (verbose_level>3 ) {
					SERIAL_ECHOPAIR("x: ", X_current);
					SERIAL_ECHOPAIR("y: ", Y_current);
					SERIAL_PROTOCOLLNPGM("");
				}

				do_blocking_move_to( X_current, Y_current, Z_current );
			}
			do_blocking_move_to( X_probe_location, Y_probe_location, Z_start_location); // Go back to the probe location
		}

		if (engage_probe_for_each_reading)  {
        		engage_z_probe();	
          		delay(1000);
		}

		setup_for_endstop_move();
                run_z_probe();

		sample_set[n] = current_position[Z_AXIS];

//
// Get the current mean for the data points we have so far
//
		sum=0.0; 
		for( j=0; j<=n; j++) {
			sum = sum + sample_set[j];
		}
		mean = sum / (double (n+1));
//
// Now, use that mean to calculate the standard deviation for the
// data points we have so far
//

		sum=0.0; 
		for( j=0; j<=n; j++) {
			sum = sum + (sample_set[j]-mean) * (sample_set[j]-mean);
		}
		sigma = sqrt( sum / (double (n+1)) );

		if (verbose_level > 1) {
			SERIAL_PROTOCOL(n+1);
			SERIAL_PROTOCOL(" of ");
			SERIAL_PROTOCOL(n_samples);
			SERIAL_PROTOCOLPGM("   z: ");
			SERIAL_PROTOCOL_F(current_position[Z_AXIS], 6);
		}

		if (verbose_level > 2) {
			SERIAL_PROTOCOL(" mean: ");
			SERIAL_PROTOCOL_F(mean,6);

			SERIAL_PROTOCOL("   sigma: ");
			SERIAL_PROTOCOL_F(sigma,6);
		}

		if (verbose_level > 0) 
			SERIAL_PROTOCOLPGM("\n");

		plan_buffer_line( X_probe_location, Y_probe_location, Z_start_location, 
				  current_position[E_AXIS], homing_feedrate[Z_AXIS]/60, active_extruder);
        	st_synchronize();

		if (engage_probe_for_each_reading)  {
        		retract_z_probe();	
          		delay(1000);
		}
	}

        retract_z_probe();
	delay(1000);

        clean_up_after_endstop_move();

//      enable_endstops(true);

	if (verbose_level > 0) {
		SERIAL_PROTOCOLPGM("Mean: ");
		SERIAL_PROTOCOL_F(mean, 6);
		SERIAL_PROTOCOLPGM("\n");
	}

SERIAL_PROTOCOLPGM("Standard Deviation: ");
SERIAL_PROTOCOL_F(sigma, 6);
SERIAL_PROTOCOLPGM("\n\n");

Sigma_Exit:
        break;
	}
#endif		// Z_PROBE_REPEATABILITY_TEST 
#endif		// ENABLE_AUTO_BED_LEVELING

    case 104: // M104
      if(setTargetedHotend(104)){
        break;
      }
      if (code_seen('S')) setTargetHotend(code_value(), tmp_extruder);

      setWatch();
      break;
    case 112: //  M112 -Emergency Stop
      kill();
      break;
    case 140: // M140 set bed temp
      if (code_seen('S')) setTargetBed(code_value());
      break;
    case 105 : // M105
      if(setTargetedHotend(105)){
        break;
        }
      #if defined(TEMP_0_PIN) && TEMP_0_PIN > -1
        SERIAL_PROTOCOLPGM("ok T:");
        SERIAL_PROTOCOL_F(degHotend(tmp_extruder),1);
        SERIAL_PROTOCOLPGM(" /");
        SERIAL_PROTOCOL_F(degTargetHotend(tmp_extruder),1);
        #if defined(TEMP_BED_PIN) && TEMP_BED_PIN > -1
          SERIAL_PROTOCOLPGM(" B:");
          SERIAL_PROTOCOL_F(degBed(),1);
          SERIAL_PROTOCOLPGM(" /");
          SERIAL_PROTOCOL_F(degTargetBed(),1);
        #endif //TEMP_BED_PIN
        for (int8_t cur_extruder = 0; cur_extruder < EXTRUDERS; ++cur_extruder) {
          SERIAL_PROTOCOLPGM(" T");
          SERIAL_PROTOCOL(cur_extruder);
          SERIAL_PROTOCOLPGM(":");
          SERIAL_PROTOCOL_F(degHotend(cur_extruder),1);
          SERIAL_PROTOCOLPGM(" /");
          SERIAL_PROTOCOL_F(degTargetHotend(cur_extruder),1);
        }
      #else
        SERIAL_ERROR_START;
        SERIAL_ERRORLNPGM(MSG_ERR_NO_THERMISTORS);
      #endif

        SERIAL_PROTOCOLPGM(" @:");
      #ifdef EXTRUDER_WATTS
        SERIAL_PROTOCOL((EXTRUDER_WATTS * getHeaterPower(tmp_extruder))/127);
        SERIAL_PROTOCOLPGM("W");
      #else
        SERIAL_PROTOCOL(getHeaterPower(tmp_extruder));
      #endif

        SERIAL_PROTOCOLPGM(" B@:");
      #ifdef BED_WATTS
        SERIAL_PROTOCOL((BED_WATTS * getHeaterPower(-1))/127);
        SERIAL_PROTOCOLPGM("W");
      #else
        SERIAL_PROTOCOL(getHeaterPower(-1));
      #endif

        #ifdef SHOW_TEMP_ADC_VALUES
          #if defined(TEMP_BED_PIN) && TEMP_BED_PIN > -1
            SERIAL_PROTOCOLPGM("    ADC B:");
            SERIAL_PROTOCOL_F(degBed(),1);
            SERIAL_PROTOCOLPGM("C->");
            SERIAL_PROTOCOL_F(rawBedTemp()/OVERSAMPLENR,0);
          #endif
          for (int8_t cur_extruder = 0; cur_extruder < EXTRUDERS; ++cur_extruder) {
            SERIAL_PROTOCOLPGM("  T");
            SERIAL_PROTOCOL(cur_extruder);
            SERIAL_PROTOCOLPGM(":");
            SERIAL_PROTOCOL_F(degHotend(cur_extruder),1);
            SERIAL_PROTOCOLPGM("C->");
            SERIAL_PROTOCOL_F(rawHotendTemp(cur_extruder)/OVERSAMPLENR,0);
          }
        #endif

        SERIAL_PROTOCOLLN("");
      return;
      break;
    case 109:
    {// M109 - Wait for extruder heater to reach target.
      if(setTargetedHotend(109)){
        break;
      }

      #ifdef AUTOTEMP
        autotemp_enabled=false;
      #endif
      if (code_seen('S')) {
        setTargetHotend(code_value(), tmp_extruder);
        CooldownNoWait = true;
      } else if (code_seen('R')) {
        setTargetHotend(code_value(), tmp_extruder);
        CooldownNoWait = false;
      }
      #ifdef AUTOTEMP
        if (code_seen('S')) autotemp_min=code_value();
        if (code_seen('B')) autotemp_max=code_value();
        if (code_seen('F'))
        {
          autotemp_factor=code_value();
          autotemp_enabled=true;
        }
      #endif

      setWatch();
      codenum = millis();

      /* See if we are heating up or cooling down */
      target_direction = isHeatingHotend(tmp_extruder); // true if heating, false if cooling

      cancel_heatup = false;

      #ifdef TEMP_RESIDENCY_TIME
        long residencyStart;
        residencyStart = -1;
        /* continue to loop until we have reached the target temp
          _and_ until TEMP_RESIDENCY_TIME hasn't passed since we reached it */
        while((!cancel_heatup)&&((residencyStart == -1) ||
              (residencyStart >= 0 && (((unsigned int) (millis() - residencyStart)) < (TEMP_RESIDENCY_TIME * 1000UL)))) ) {
      #else
        while ( target_direction ? (isHeatingHotend(tmp_extruder)) : (isCoolingHotend(tmp_extruder)&&(CooldownNoWait==false)) ) {
      #endif //TEMP_RESIDENCY_TIME
          if( (millis() - codenum) > 1000UL )
          { //Print Temp Reading and remaining time every 1 second while heating up/cooling down
            SERIAL_PROTOCOLPGM("T:");
            SERIAL_PROTOCOL_F(degHotend(tmp_extruder),1);
            SERIAL_PROTOCOLPGM(" E:");
            SERIAL_PROTOCOL((int)tmp_extruder);
            #ifdef TEMP_RESIDENCY_TIME
              SERIAL_PROTOCOLPGM(" W:");
              if(residencyStart > -1)
              {
                 codenum = ((TEMP_RESIDENCY_TIME * 1000UL) - (millis() - residencyStart)) / 1000UL;
                 SERIAL_PROTOCOLLN( codenum );
              }
              else
              {
                 SERIAL_PROTOCOLLN( "?" );
              }
            #else
              SERIAL_PROTOCOLLN("");
            #endif
            codenum = millis();
          }
          manage_heater();
          manage_inactivity();

        #ifdef TEMP_RESIDENCY_TIME
            /* start/restart the TEMP_RESIDENCY_TIME timer whenever we reach target temp for the first time
              or when current temp falls outside the hysteresis after target temp was reached */
          if ((residencyStart == -1 &&  target_direction && (degHotend(tmp_extruder) >= (degTargetHotend(tmp_extruder)-TEMP_WINDOW))) ||
              (residencyStart == -1 && !target_direction && (degHotend(tmp_extruder) <= (degTargetHotend(tmp_extruder)+TEMP_WINDOW))) ||
              (residencyStart > -1 && labs(degHotend(tmp_extruder) - degTargetHotend(tmp_extruder)) > TEMP_HYSTERESIS) )
          {
            residencyStart = millis();
          }
        #endif //TEMP_RESIDENCY_TIME
        }

        starttime=millis();
        previous_millis_cmd = millis();
      }
      break;
    case 190: // M190 - Wait for bed heater to reach target.
    #if defined(TEMP_BED_PIN) && TEMP_BED_PIN > -1

        if (code_seen('S')) {
          setTargetBed(code_value());
          CooldownNoWait = true;
        } else if (code_seen('R')) {
          setTargetBed(code_value());
          CooldownNoWait = false;
        }
        codenum = millis();
        
        cancel_heatup = false;
        target_direction = isHeatingBed(); // true if heating, false if cooling

        while ( (target_direction)&&(!cancel_heatup) ? (isHeatingBed()) : (isCoolingBed()&&(CooldownNoWait==false)) )
        {
          if(( millis() - codenum) > 1000 ) //Print Temp Reading every 1 second while heating up.
          {
            float tt=degHotend(active_extruder);
            SERIAL_PROTOCOLPGM("T:");
            SERIAL_PROTOCOL(tt);
            SERIAL_PROTOCOLPGM(" E:");
            SERIAL_PROTOCOL((int)active_extruder);
            SERIAL_PROTOCOLPGM(" B:");
            SERIAL_PROTOCOL_F(degBed(),1);
            SERIAL_PROTOCOLLN("");
            codenum = millis();
          }
          manage_heater();
          manage_inactivity();

        }

        previous_millis_cmd = millis();
    #endif
        break;

    #if defined(FAN_PIN) && FAN_PIN > -1
      case 106: //M106 Fan On
        if (code_seen('S')){
           fanSpeed=constrain(code_value(),0,255);
        }
        else {
          fanSpeed=255;
        }
        break;
      case 107: //M107 Fan Off
        fanSpeed = 0;
        break;
    #endif //FAN_PIN

    #if defined(PS_ON_PIN) && PS_ON_PIN > -1
      case 80: // M80 - Turn on Power Supply
        SET_OUTPUT(PS_ON_PIN); //GND
        WRITE(PS_ON_PIN, PS_ON_AWAKE);

        // If you have a switch on suicide pin, this is useful
        // if you want to start another print with suicide feature after
        // a print without suicide...
        #if defined SUICIDE_PIN && SUICIDE_PIN > -1
            SET_OUTPUT(SUICIDE_PIN);
            WRITE(SUICIDE_PIN, HIGH);
        #endif

        break;
      #endif

      case 81: // M81 - Turn off Power Supply
        disable_heater();
        st_synchronize();
        disable_e0();
        disable_e1();
        disable_e2();
        finishAndDisableSteppers();
        fanSpeed = 0;
        delay(1000); // Wait a little before to switch off
      #if defined(SUICIDE_PIN) && SUICIDE_PIN > -1
        st_synchronize();
        suicide();
      #elif defined(PS_ON_PIN) && PS_ON_PIN > -1
        SET_OUTPUT(PS_ON_PIN);
        WRITE(PS_ON_PIN, PS_ON_ASLEEP);
      #endif
	  break;

    case 82:
      axis_relative_modes[3] = false;
      break;
    case 83:
      axis_relative_modes[3] = true;
      break;
    case 18: //compatibility
    case 84: // M84
      if(code_seen('S')){
        stepper_inactive_time = code_value() * 1000;
      }
      else
      {
        bool all_axis = !((code_seen(axis_codes[X_AXIS])) || (code_seen(axis_codes[Y_AXIS])) || (code_seen(axis_codes[Z_AXIS]))|| (code_seen(axis_codes[E_AXIS])));
        if(all_axis)
        {
          st_synchronize();
          disable_e0();
          disable_e1();
          disable_e2();
          finishAndDisableSteppers();
        }
        else
        {
          st_synchronize();
          if(code_seen('X')) disable_x();
          if(code_seen('Y')) disable_y();
          if(code_seen('Z')) disable_z();
          #if ((E0_ENABLE_PIN != X_ENABLE_PIN) && (E1_ENABLE_PIN != Y_ENABLE_PIN)) // Only enable on boards that have seperate ENABLE_PINS
            if(code_seen('E')) {
              disable_e0();
              disable_e1();
              disable_e2();
            }
          #endif
        }
      }
      break;
    case 85: // M85
      if(code_seen('S')) {
        max_inactive_time = code_value() * 1000;
      }
      break;
    case 92: // M92
      for(int8_t i=0; i < NUM_AXIS; i++)
      {
        if(code_seen(axis_codes[i]))
        {
          if(i == 3) { // E
            float value = code_value();
            if(value < 20.0) {
              float factor = axis_steps_per_unit[i] / value; // increase e constants if M92 E14 is given for netfab.
              max_e_jerk *= factor;
              max_feedrate[i] *= factor;
              axis_steps_per_sqr_second[i] *= factor;
            }
            axis_steps_per_unit[i] = value;
          }
          else {
            axis_steps_per_unit[i] = code_value();
          }
        }
      }
      break;
    case 115: // M115
      SERIAL_PROTOCOLPGM(MSG_M115_REPORT);
      break;
    case 114: // M114
      SERIAL_PROTOCOLPGM("X:");
      SERIAL_PROTOCOL(current_position[X_AXIS]);
      SERIAL_PROTOCOLPGM(" Y:");
      SERIAL_PROTOCOL(current_position[Y_AXIS]);
      SERIAL_PROTOCOLPGM(" Z:");
      SERIAL_PROTOCOL(current_position[Z_AXIS]);
      SERIAL_PROTOCOLPGM(" E:");
      SERIAL_PROTOCOL(current_position[E_AXIS]);

      SERIAL_PROTOCOLPGM(MSG_COUNT_X);
      SERIAL_PROTOCOL(float(st_get_position(X_AXIS))/axis_steps_per_unit[X_AXIS]);
      SERIAL_PROTOCOLPGM(" Y:");
      SERIAL_PROTOCOL(float(st_get_position(Y_AXIS))/axis_steps_per_unit[Y_AXIS]);
      SERIAL_PROTOCOLPGM(" Z:");
      SERIAL_PROTOCOL(float(st_get_position(Z_AXIS))/axis_steps_per_unit[Z_AXIS]);

      SERIAL_PROTOCOLLN("");

      break;
    case 120: // M120
      enable_endstops(false) ;
      break;
    case 121: // M121
      enable_endstops(true) ;
      break;
    case 119: // M119
    SERIAL_PROTOCOLLN(MSG_M119_REPORT);
      #if defined(X_MIN_PIN) && X_MIN_PIN > -1
        SERIAL_PROTOCOLPGM(MSG_X_MIN);
        SERIAL_PROTOCOLLN(((READ(X_MIN_PIN)^X_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
      #endif
      #if defined(X_MAX_PIN) && X_MAX_PIN > -1
        SERIAL_PROTOCOLPGM(MSG_X_MAX);
        SERIAL_PROTOCOLLN(((READ(X_MAX_PIN)^X_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
      #endif
      #if defined(Y_MIN_PIN) && Y_MIN_PIN > -1
        SERIAL_PROTOCOLPGM(MSG_Y_MIN);
        SERIAL_PROTOCOLLN(((READ(Y_MIN_PIN)^Y_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
      #endif
      #if defined(Y_MAX_PIN) && Y_MAX_PIN > -1
        SERIAL_PROTOCOLPGM(MSG_Y_MAX);
        SERIAL_PROTOCOLLN(((READ(Y_MAX_PIN)^Y_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
      #endif
      #if defined(Z_MIN_PIN) && Z_MIN_PIN > -1
        SERIAL_PROTOCOLPGM(MSG_Z_MIN);
        SERIAL_PROTOCOLLN(((READ(Z_MIN_PIN)^Z_MIN_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
      #endif
      #if defined(Z_MAX_PIN) && Z_MAX_PIN > -1
        SERIAL_PROTOCOLPGM(MSG_Z_MAX);
        SERIAL_PROTOCOLLN(((READ(Z_MAX_PIN)^Z_MAX_ENDSTOP_INVERTING)?MSG_ENDSTOP_HIT:MSG_ENDSTOP_OPEN));
      #endif
      break;
      //TODO: update for all axis, use for loop

    case 200: // M200 D<millimeters> set filament diameter and set E axis units to cubic millimeters (use S0 to set back to millimeters).
      {

        tmp_extruder = active_extruder;
        if(code_seen('T')) {
          tmp_extruder = code_value();
          if(tmp_extruder >= EXTRUDERS) {
            SERIAL_ECHO_START;
            SERIAL_ECHO(MSG_M200_INVALID_EXTRUDER);
            break;
          }
        }

        float area = .0;
        if(code_seen('D')) {
		  float diameter = (float)code_value();
		  if (diameter == 0.0) {
			// setting any extruder filament size disables volumetric on the assumption that
			// slicers either generate in extruder values as cubic mm or as as filament feeds
			// for all extruders
		    volumetric_enabled = false;
		  } else {
            filament_size[tmp_extruder] = (float)code_value();
			// make sure all extruders have some sane value for the filament size
			filament_size[0] = (filament_size[0] == 0.0 ? DEFAULT_NOMINAL_FILAMENT_DIA : filament_size[0]);
            #if EXTRUDERS > 1
			filament_size[1] = (filament_size[1] == 0.0 ? DEFAULT_NOMINAL_FILAMENT_DIA : filament_size[1]);
            #if EXTRUDERS > 2
			filament_size[2] = (filament_size[2] == 0.0 ? DEFAULT_NOMINAL_FILAMENT_DIA : filament_size[2]);
            #endif
            #endif
			volumetric_enabled = true;
		  }
        } else {
          //reserved for setting filament diameter via UFID or filament measuring device
          break;
        }
		calculate_volumetric_multipliers();
      }
      break;
    case 201: // M201
      for(int8_t i=0; i < NUM_AXIS; i++)
      {
        if(code_seen(axis_codes[i]))
        {
          max_acceleration_units_per_sq_second[i] = code_value();
        }
      }
      // steps per sq second need to be updated to agree with the units per sq second (as they are what is used in the planner)
      reset_acceleration_rates();
      break;
    #if 0 // Not used for Sprinter/grbl gen6
    case 202: // M202
      for(int8_t i=0; i < NUM_AXIS; i++) {
        if(code_seen(axis_codes[i])) axis_travel_steps_per_sqr_second[i] = code_value() * axis_steps_per_unit[i];
      }
      break;
    #endif
    case 203: // M203 max feedrate mm/sec
      for(int8_t i=0; i < NUM_AXIS; i++) {
        if(code_seen(axis_codes[i])) max_feedrate[i] = code_value();
      }
      break;
    case 204: // M204 acclereration S normal moves T filmanent only moves
      {
        if(code_seen('S')) acceleration = code_value() ;
        if(code_seen('T')) retract_acceleration = code_value() ;
      }
      break;
    case 205: //M205 advanced settings:  minimum travel speed S=while printing T=travel only,  B=minimum segment time X= maximum xy jerk, Z=maximum Z jerk
    {
      if(code_seen('S')) minimumfeedrate = code_value();
      if(code_seen('T')) mintravelfeedrate = code_value();
      if(code_seen('B')) minsegmenttime = code_value() ;
      if(code_seen('X')) max_xy_jerk = code_value() ;
      if(code_seen('Z')) max_z_jerk = code_value() ;
      if(code_seen('E')) max_e_jerk = code_value() ;
    }
    break;
    case 206: // M206 additional homing offset
      for(int8_t i=0; i < 3; i++)
      {
        if(code_seen(axis_codes[i])) add_homing[i] = code_value();
      }
      break;


    #if EXTRUDERS > 1
    case 218: // M218 - set hotend offset (in mm), T<extruder_number> X<offset_on_X> Y<offset_on_Y>
    {
      if(setTargetedHotend(218)){
        break;
      }
      if(code_seen('X'))
      {
        extruder_offset[X_AXIS][tmp_extruder] = code_value();
      }
      if(code_seen('Y'))
      {
        extruder_offset[Y_AXIS][tmp_extruder] = code_value();
      }
      SERIAL_ECHO_START;
      SERIAL_ECHOPGM(MSG_HOTEND_OFFSET);
      for(tmp_extruder = 0; tmp_extruder < EXTRUDERS; tmp_extruder++)
      {
         SERIAL_ECHO(" ");
         SERIAL_ECHO(extruder_offset[X_AXIS][tmp_extruder]);
         SERIAL_ECHO(",");
         SERIAL_ECHO(extruder_offset[Y_AXIS][tmp_extruder]);
      }
      SERIAL_ECHOLN("");
    }break;
    #endif
    case 220: // M220 S<factor in percent>- set speed factor override percentage
    {
      if(code_seen('S'))
      {
        feedmultiply = code_value() ;
      }
    }
    break;
    case 221: // M221 S<factor in percent>- set extrude factor override percentage
    {
      if(code_seen('S'))
      {
        int tmp_code = code_value();
        if (code_seen('T'))
        {
          if(setTargetedHotend(221)){
            break;
          }
          extruder_multiply[tmp_extruder] = tmp_code;
        }
        else
        {
          extrudemultiply = tmp_code ;
        }
      }
    }
    break;

	case 226: // M226 P<pin number> S<pin state>- Wait until the specified pin reaches the state required
	{
      if(code_seen('P')){
        int pin_number = code_value(); // pin number
        int pin_state = -1; // required pin state - default is inverted

        if(code_seen('S')) pin_state = code_value(); // required pin state

        if(pin_state >= -1 && pin_state <= 1){

          for(int8_t i = 0; i < (int8_t)(sizeof(sensitive_pins)/sizeof(int)); i++)
          {
            if (sensitive_pins[i] == pin_number)
            {
              pin_number = -1;
              break;
            }
          }

          if (pin_number > -1)
          {
            int target = LOW;

            st_synchronize();

            pinMode(pin_number, INPUT);

            switch(pin_state){
            case 1:
              target = HIGH;
              break;

            case 0:
              target = LOW;
              break;

            case -1:
              target = !digitalRead(pin_number);
              break;
            }

            while(digitalRead(pin_number) != target){
              manage_heater();
              manage_inactivity();
            }
          }
        }
      }
    }
    break;

    #if (LARGE_FLASH == true && ( BEEPER > 0 || defined(ULTRALCD) || defined(LCD_USE_I2C_BUZZER)))
    case 300: // M300
    {
      int beepS = code_seen('S') ? code_value() : 110;
      int beepP = code_seen('P') ? code_value() : 1000;
      if (beepS > 0)
      {
        #if BEEPER > 0
          tone(BEEPER, beepS);
          delay(beepP);
          noTone(BEEPER);
        #elif defined(ULTRALCD)
		  lcd_buzz(beepS, beepP);
		#elif defined(LCD_USE_I2C_BUZZER)
		  lcd_buzz(beepP, beepS);
        #endif
      }
      else
      {
        delay(beepP);
      }
    }
    break;
    #endif // M300

    #ifdef PIDTEMP
    case 301: // M301
      {
        if(code_seen('P')) Kp = code_value();
        if(code_seen('I')) Ki = scalePID_i(code_value());
        if(code_seen('D')) Kd = scalePID_d(code_value());

        #ifdef PID_ADD_EXTRUSION_RATE
        if(code_seen('C')) Kc = code_value();
        #endif

        updatePID();
        SERIAL_PROTOCOL(MSG_OK);
        SERIAL_PROTOCOL(" p:");
        SERIAL_PROTOCOL(Kp);
        SERIAL_PROTOCOL(" i:");
        SERIAL_PROTOCOL(unscalePID_i(Ki));
        SERIAL_PROTOCOL(" d:");
        SERIAL_PROTOCOL(unscalePID_d(Kd));
        #ifdef PID_ADD_EXTRUSION_RATE
        SERIAL_PROTOCOL(" c:");
        //Kc does not have scaling applied above, or in resetting defaults
        SERIAL_PROTOCOL(Kc);
        #endif
        SERIAL_PROTOCOLLN("");
      }
      break;
    #endif //PIDTEMP
    #ifdef PIDTEMPBED
    case 304: // M304
      {
        if(code_seen('P')) bedKp = code_value();
        if(code_seen('I')) bedKi = scalePID_i(code_value());
        if(code_seen('D')) bedKd = scalePID_d(code_value());

        updatePID();
        SERIAL_PROTOCOL(MSG_OK);
        SERIAL_PROTOCOL(" p:");
        SERIAL_PROTOCOL(bedKp);
        SERIAL_PROTOCOL(" i:");
        SERIAL_PROTOCOL(unscalePID_i(bedKi));
        SERIAL_PROTOCOL(" d:");
        SERIAL_PROTOCOL(unscalePID_d(bedKd));
        SERIAL_PROTOCOLLN("");
      }
      break;
    #endif //PIDTEMP
    

    #ifdef PREVENT_DANGEROUS_EXTRUDE
    case 302: // allow cold extrudes, or set the minimum extrude temperature
    {
	  float temp = .0;
	  if (code_seen('S')) temp=code_value();
      set_extrude_min_temp(temp);
    }
    break;
	#endif
    case 303: // M303 PID autotune
    {
      float temp = 150.0;
      int e=0;
      int c=5;
      if (code_seen('E')) e=code_value();
        if (e<0)
          temp=70;
      if (code_seen('S')) temp=code_value();
      if (code_seen('C')) c=code_value();
      PID_autotune(temp, e, c);
    }
    break;

    case 400: // M400 finish all moves
    {
      st_synchronize();
    }
    break;

    case 401:
    {
        engage_z_probe();    // Engage Z endstop if available
    }
    break;

    case 402:
    {
        retract_z_probe();    // Retract Z endstop if enabled
    }
    break;

    case 500: // M500 Store settings in EEPROM
    {
        Config_StoreSettings();
    }
    break;
    case 501: // M501 Read settings from EEPROM
    {
        Config_RetrieveSettings();
    }
    break;
    case 502: // M502 Revert to default settings
    {
        Config_ResetDefault();
    }
    break;
    case 503: // M503 print settings currently in memory
    {
        Config_PrintSettings();
    }
    break;

    #ifdef CUSTOM_M_CODE_SET_Z_PROBE_OFFSET
    case CUSTOM_M_CODE_SET_Z_PROBE_OFFSET:
    {
      float value;
      if (code_seen('Z'))
      {
        value = code_value();
        if ((Z_PROBE_OFFSET_RANGE_MIN <= value) && (value <= Z_PROBE_OFFSET_RANGE_MAX))
        {
          zprobe_zoffset = -value; // compare w/ line 278 of ConfigurationStore.cpp
          SERIAL_ECHO_START;
          SERIAL_ECHOLNPGM(MSG_ZPROBE_ZOFFSET " " MSG_OK);
          SERIAL_PROTOCOLLN("");
        }
        else
        {
          SERIAL_ECHO_START;
          SERIAL_ECHOPGM(MSG_ZPROBE_ZOFFSET);
          SERIAL_ECHOPGM(MSG_Z_MIN);
          SERIAL_ECHO(Z_PROBE_OFFSET_RANGE_MIN);
          SERIAL_ECHOPGM(MSG_Z_MAX);
          SERIAL_ECHO(Z_PROBE_OFFSET_RANGE_MAX);
          SERIAL_PROTOCOLLN("");
        }
      }
      else
      {
          SERIAL_ECHO_START;
          SERIAL_ECHOLNPGM(MSG_ZPROBE_ZOFFSET " : ");
          SERIAL_ECHO(-zprobe_zoffset);
          SERIAL_PROTOCOLLN("");
      }
      break;
    }
    #endif // CUSTOM_M_CODE_SET_Z_PROBE_OFFSET

    #ifdef FILAMENTCHANGEENABLE
    case 600: //Pause for filament change X[pos] Y[pos] Z[relative lift] E[initial retract] L[later retract distance for removal]
    {
        float target[4];
        float lastpos[4];
        target[X_AXIS]=current_position[X_AXIS];
        target[Y_AXIS]=current_position[Y_AXIS];
        target[Z_AXIS]=current_position[Z_AXIS];
        target[E_AXIS]=current_position[E_AXIS];
        lastpos[X_AXIS]=current_position[X_AXIS];
        lastpos[Y_AXIS]=current_position[Y_AXIS];
        lastpos[Z_AXIS]=current_position[Z_AXIS];
        lastpos[E_AXIS]=current_position[E_AXIS];
        //retract by E
        if(code_seen('E'))
        {
          target[E_AXIS]+= code_value();
        }
        else
        {
          #ifdef FILAMENTCHANGE_FIRSTRETRACT
            target[E_AXIS]+= FILAMENTCHANGE_FIRSTRETRACT ;
          #endif
        }
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

        //lift Z
        if(code_seen('Z'))
        {
          target[Z_AXIS]+= code_value();
        }
        else
        {
          #ifdef FILAMENTCHANGE_ZADD
            target[Z_AXIS]+= FILAMENTCHANGE_ZADD ;
          #endif
        }
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

        //move xy
        if(code_seen('X'))
        {
          target[X_AXIS]+= code_value();
        }
        else
        {
          #ifdef FILAMENTCHANGE_XPOS
            target[X_AXIS]= FILAMENTCHANGE_XPOS ;
          #endif
        }
        if(code_seen('Y'))
        {
          target[Y_AXIS]= code_value();
        }
        else
        {
          #ifdef FILAMENTCHANGE_YPOS
            target[Y_AXIS]= FILAMENTCHANGE_YPOS ;
          #endif
        }

        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

        if(code_seen('L'))
        {
          target[E_AXIS]+= code_value();
        }
        else
        {
          #ifdef FILAMENTCHANGE_FINALRETRACT
            target[E_AXIS]+= FILAMENTCHANGE_FINALRETRACT ;
          #endif
        }

        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder);

        //finish moves
        st_synchronize();
        //disable extruder steppers so filament can be removed
        disable_e0();
        disable_e1();
        disable_e2();
        delay(100);
        LCD_ALERTMESSAGEPGM(MSG_FILAMENTCHANGE);
        uint8_t cnt=0;
        while(!lcd_clicked()){
          cnt++;
          manage_heater();
          manage_inactivity(true);
          if(cnt==0)
          {
          #if BEEPER > 0
            SET_OUTPUT(BEEPER);

            WRITE(BEEPER,HIGH);
            delay(3);
            WRITE(BEEPER,LOW);
            delay(3);
          #else
			#if !defined(LCD_FEEDBACK_FREQUENCY_HZ) || !defined(LCD_FEEDBACK_FREQUENCY_DURATION_MS)
              lcd_buzz(1000/6,100);
			#else
			  lcd_buzz(LCD_FEEDBACK_FREQUENCY_DURATION_MS,LCD_FEEDBACK_FREQUENCY_HZ);
			#endif
          #endif
          }
        }

        //return to normal
        if(code_seen('L'))
        {
          target[E_AXIS]+= -code_value();
        }
        else
        {
          #ifdef FILAMENTCHANGE_FINALRETRACT
            target[E_AXIS]+=(-1)*FILAMENTCHANGE_FINALRETRACT ;
          #endif
        }
        current_position[E_AXIS]=target[E_AXIS]; //the long retract of L is compensated by manual filament feeding
        plan_set_e_position(current_position[E_AXIS]);
        plan_buffer_line(target[X_AXIS], target[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //should do nothing
        plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], target[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //move xy back
        plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], target[E_AXIS], feedrate/60, active_extruder); //move z back
        plan_buffer_line(lastpos[X_AXIS], lastpos[Y_AXIS], lastpos[Z_AXIS], lastpos[E_AXIS], feedrate/60, active_extruder); //final untretract
    }
    break;
    #endif //FILAMENTCHANGEENABLE

    case 999: // M999: Restart after being stopped
      Stopped = false;

      gcode_LastN = Stopped_gcode_LastN;
      FlushSerialRequestResend();
    break;
    }
  }

  else if(code_seen('T'))
  {
    tmp_extruder = code_value();
    if(tmp_extruder >= EXTRUDERS) {
      SERIAL_ECHO_START;
      SERIAL_ECHO("T");
      SERIAL_ECHO(tmp_extruder);
      SERIAL_ECHOLN(MSG_INVALID_EXTRUDER);
    }
    else {
      boolean make_move = false;
      if(code_seen('F')) {
        make_move = true;
        next_feedrate = code_value();
        if(next_feedrate > 0.0) {
          feedrate = next_feedrate;
        }
      }
      #if EXTRUDERS > 1
      if(tmp_extruder != active_extruder) {
        // Save current position to return to after applying extruder offset
        memcpy(destination, current_position, sizeof(destination));

        // Offset extruder (only by XY)
        int i;
        for(i = 0; i < 2; i++) {
           current_position[i] = current_position[i] -
                                 extruder_offset[i][active_extruder] +
                                 extruder_offset[i][tmp_extruder];
        }
        // Set the new active extruder and position
        active_extruder = tmp_extruder;


        plan_set_position(current_position[X_AXIS], current_position[Y_AXIS], current_position[Z_AXIS], current_position[E_AXIS]);

        // Move to the old position if 'F' was in the parameters
        if(make_move && Stopped == false) {
           prepare_move();
        }
      }
      #endif
      SERIAL_ECHO_START;
      SERIAL_ECHO(MSG_ACTIVE_EXTRUDER);
      SERIAL_PROTOCOLLN((int)active_extruder);
    }
  }

  else
  {
    SERIAL_ECHO_START;
    SERIAL_ECHOPGM(MSG_UNKNOWN_COMMAND);
    SERIAL_ECHO(cmdbuffer[bufindr]);
    SERIAL_ECHOLNPGM("\"");
  }

  ClearToSend();
}

void FlushSerialRequestResend()
{
  //char cmdbuffer[bufindr][100]="Resend:";
  MYSERIAL.flush();
  SERIAL_PROTOCOLPGM(MSG_RESEND);
  SERIAL_PROTOCOLLN(gcode_LastN + 1);
  ClearToSend();
}

void ClearToSend()
{
  previous_millis_cmd = millis();
  SERIAL_PROTOCOLLNPGM(MSG_OK);
}

void get_coordinates()
{
  bool seen[4]={false,false,false,false};
  for(int8_t i=0; i < NUM_AXIS; i++) {
    if(code_seen(axis_codes[i]))
    {
      destination[i] = (float)code_value() + (axis_relative_modes[i] || relative_mode)*current_position[i];
      seen[i]=true;
    }
    else destination[i] = current_position[i]; //Are these else lines really needed?
  }
  if(code_seen('F')) {
    next_feedrate = code_value();
    if(next_feedrate > 0.0) feedrate = next_feedrate;
  }
}

void get_arc_coordinates()
{
#ifdef SF_ARC_FIX
   bool relative_mode_backup = relative_mode;
   relative_mode = true;
#endif
   get_coordinates();
#ifdef SF_ARC_FIX
   relative_mode=relative_mode_backup;
#endif

   if(code_seen('I')) {
     offset[0] = code_value();
   }
   else {
     offset[0] = 0.0;
   }
   if(code_seen('J')) {
     offset[1] = code_value();
   }
   else {
     offset[1] = 0.0;
   }
}

void clamp_to_software_endstops(float target[3])
{
  if (min_software_endstops) {
    if (target[X_AXIS] < min_pos[X_AXIS]) target[X_AXIS] = min_pos[X_AXIS];
    if (target[Y_AXIS] < min_pos[Y_AXIS]) target[Y_AXIS] = min_pos[Y_AXIS];
    
    float negative_z_offset = 0;
    #ifdef ENABLE_AUTO_BED_LEVELING
      if (Z_PROBE_OFFSET_FROM_EXTRUDER < 0) negative_z_offset = negative_z_offset + Z_PROBE_OFFSET_FROM_EXTRUDER;
      if (add_homing[Z_AXIS] < 0) negative_z_offset = negative_z_offset + add_homing[Z_AXIS];
    #endif
    
    if (target[Z_AXIS] < min_pos[Z_AXIS]+negative_z_offset) target[Z_AXIS] = min_pos[Z_AXIS]+negative_z_offset;
  }

  if (max_software_endstops) {
    if (target[X_AXIS] > max_pos[X_AXIS]) target[X_AXIS] = max_pos[X_AXIS];
    if (target[Y_AXIS] > max_pos[Y_AXIS]) target[Y_AXIS] = max_pos[Y_AXIS];
    if (target[Z_AXIS] > max_pos[Z_AXIS]) target[Z_AXIS] = max_pos[Z_AXIS];
  }
}



void prepare_move()
{
  clamp_to_software_endstops(destination);
  previous_millis_cmd = millis();

  // Do not use feedmultiply for E or Z only moves
  if( (current_position[X_AXIS] == destination [X_AXIS]) && (current_position[Y_AXIS] == destination [Y_AXIS])) {
      plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate/60, active_extruder);
  }
  else {
    plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS], destination[E_AXIS], feedrate*feedmultiply/60/100.0, active_extruder);
  }

  for(int8_t i=0; i < NUM_AXIS; i++) {
    current_position[i] = destination[i];
  }
}

void prepare_arc_move(char isclockwise) {
  float r = hypot(offset[X_AXIS], offset[Y_AXIS]); // Compute arc radius for mc_arc

  // Trace the arc
  mc_arc(current_position, destination, offset, X_AXIS, Y_AXIS, Z_AXIS, feedrate*feedmultiply/60/100.0, r, isclockwise, active_extruder);

  // As far as the parser is concerned, the position is now == target. In reality the
  // motion control system might still be processing the action and the real tool position
  // in any intermediate location.
  for(int8_t i=0; i < NUM_AXIS; i++) {
    current_position[i] = destination[i];
  }
  previous_millis_cmd = millis();
}

#if defined(CONTROLLERFAN_PIN) && CONTROLLERFAN_PIN > -1

#if defined(FAN_PIN)
  #if CONTROLLERFAN_PIN == FAN_PIN
    #error "You cannot set CONTROLLERFAN_PIN equal to FAN_PIN"
  #endif
#endif

unsigned long lastMotor = 0; //Save the time for when a motor was turned on last
unsigned long lastMotorCheck = 0;

void controllerFan()
{
  if ((millis() - lastMotorCheck) >= 2500) //Not a time critical function, so we only check every 2500ms
  {
    lastMotorCheck = millis();

    if(!READ(X_ENABLE_PIN) || !READ(Y_ENABLE_PIN) || !READ(Z_ENABLE_PIN) || (soft_pwm_bed > 0)
    #if EXTRUDERS > 2
       || !READ(E2_ENABLE_PIN)
    #endif
    #if EXTRUDER > 1
      #if defined(X2_ENABLE_PIN) && X2_ENABLE_PIN > -1
       || !READ(X2_ENABLE_PIN)
      #endif
       || !READ(E1_ENABLE_PIN)
    #endif
       || !READ(E0_ENABLE_PIN)) //If any of the drivers are enabled...
    {
      lastMotor = millis(); //... set time to NOW so the fan will turn on
    }

    if ((millis() - lastMotor) >= (CONTROLLERFAN_SECS*1000UL) || lastMotor == 0) //If the last time any driver was enabled, is longer since than CONTROLLERSEC...
    {
        digitalWrite(CONTROLLERFAN_PIN, 0);
        analogWrite(CONTROLLERFAN_PIN, 0);
    }
    else
    {
        // allows digital or PWM fan output to be used (see M42 handling)
        digitalWrite(CONTROLLERFAN_PIN, CONTROLLERFAN_SPEED);
        analogWrite(CONTROLLERFAN_PIN, CONTROLLERFAN_SPEED);
    }
  }
}
#endif

#ifdef TEMP_STAT_LEDS
static bool blue_led = false;
static bool red_led = false;
static uint32_t stat_update = 0;

void handle_status_leds(void) {
  float max_temp = 0.0;
  if(millis() > stat_update) {
    stat_update += 500; // Update every 0.5s
    for (int8_t cur_extruder = 0; cur_extruder < EXTRUDERS; ++cur_extruder) {
       max_temp = max(max_temp, degHotend(cur_extruder));
       max_temp = max(max_temp, degTargetHotend(cur_extruder));
    }
    #if defined(TEMP_BED_PIN) && TEMP_BED_PIN > -1
      max_temp = max(max_temp, degTargetBed());
      max_temp = max(max_temp, degBed());
    #endif
    if((max_temp > 55.0) && (red_led == false)) {
      digitalWrite(STAT_LED_RED, 1);
      digitalWrite(STAT_LED_BLUE, 0);
      red_led = true;
      blue_led = false;
    }
    if((max_temp < 54.0) && (blue_led == false)) {
      digitalWrite(STAT_LED_RED, 0);
      digitalWrite(STAT_LED_BLUE, 1);
      red_led = false;
      blue_led = true;
    }
  }
}
#endif

void manage_inactivity(bool ignore_stepper_queue/*=false*/) //default argument set in Marlin.h
{
	
#if defined(KILL_PIN) && KILL_PIN > -1
	static int killCount = 0;   // make the inactivity button a bit less responsive
   const int KILL_DELAY = 10000;
#endif

#if defined(HOME_PIN) && HOME_PIN > -1
   static int homeDebounceCount = 0;   // poor man's debouncing count
   const int HOME_DEBOUNCE_DELAY = 10000;
#endif
   
	
  if(buflen < (BUFSIZE-1))
    get_command();

  if( (millis() - previous_millis_cmd) >  max_inactive_time )
    if(max_inactive_time)
      kill();
  if(stepper_inactive_time)  {
    if( (millis() - previous_millis_cmd) >  stepper_inactive_time )
    {
      if(blocks_queued() == false && ignore_stepper_queue == false) {
        disable_x();
        disable_y();
        disable_z();
        disable_e0();
        disable_e1();
        disable_e2();
      }
    }
  }
  
  #if defined(KILL_PIN) && KILL_PIN > -1
    
    // Check if the kill button was pressed and wait just in case it was an accidental
    // key kill key press
    // -------------------------------------------------------------------------------
    if( 0 == READ(KILL_PIN) )
    {
       killCount++;
    }
    else if (killCount > 0)
    {
       killCount--;
    }
    // Exceeded threshold and we can confirm that it was not accidental
    // KILL the machine
    // ----------------------------------------------------------------
    if ( killCount >= KILL_DELAY)
    {
       kill();
    }
  #endif

#if defined(HOME_PIN) && HOME_PIN > -1
    // Check to see if we have to home, use poor man's debouncer
    // ---------------------------------------------------------
    if ( 0 == READ(HOME_PIN) )
    {
       if (homeDebounceCount == 0)
       {
          enquecommand_P((PSTR("G28")));
          homeDebounceCount++;
          LCD_ALERTMESSAGEPGM(MSG_AUTO_HOME);
       }
       else if (homeDebounceCount < HOME_DEBOUNCE_DELAY)
       {
          homeDebounceCount++;
       }
       else
       {
          homeDebounceCount = 0;
       }
    }
#endif
    
  #if defined(CONTROLLERFAN_PIN) && CONTROLLERFAN_PIN > -1
    controllerFan(); //Check if fan should be turned on to cool stepper drivers down
  #endif
  
  #ifdef EXTRUDER_RUNOUT_PREVENT
    if( (millis() - previous_millis_cmd) >  EXTRUDER_RUNOUT_SECONDS*1000 )
    if(degHotend(active_extruder)>EXTRUDER_RUNOUT_MINTEMP)
    {
     bool oldstatus=READ(E0_ENABLE_PIN);
     enable_e0();
     float oldepos=current_position[E_AXIS];
     float oldedes=destination[E_AXIS];
     plan_buffer_line(destination[X_AXIS], destination[Y_AXIS], destination[Z_AXIS],
                      destination[E_AXIS]+EXTRUDER_RUNOUT_EXTRUDE*EXTRUDER_RUNOUT_ESTEPS/axis_steps_per_unit[E_AXIS],
                      EXTRUDER_RUNOUT_SPEED/60.*EXTRUDER_RUNOUT_ESTEPS/axis_steps_per_unit[E_AXIS], active_extruder);
     current_position[E_AXIS]=oldepos;
     destination[E_AXIS]=oldedes;
     plan_set_e_position(oldepos);
     previous_millis_cmd=millis();
     st_synchronize();
     WRITE(E0_ENABLE_PIN,oldstatus);
    }
  #endif
  
  #ifdef TEMP_STAT_LEDS
      handle_status_leds();
  #endif
  
  check_axes_activity();
}

void kill()
{
  cli(); // Stop interrupts
  disable_heater();

  disable_x();
  disable_y();
  disable_z();
  disable_e0();
  disable_e1();
  disable_e2();

#if defined(PS_ON_PIN) && PS_ON_PIN > -1
  pinMode(PS_ON_PIN,INPUT);
#endif
  SERIAL_ERROR_START;
  SERIAL_ERRORLNPGM(MSG_ERR_KILLED);


  sei();   // enable interrupts

  cli();   // disable interrupts
  suicide();
  while(1) { /* Intentionally left empty */ } // Wait for reset
}

void Stop()
{
  disable_heater();
  if(Stopped == false) {
    Stopped = true;
    Stopped_gcode_LastN = gcode_LastN; // Save last g_code for restart
    SERIAL_ERROR_START;
    SERIAL_ERRORLNPGM(MSG_ERR_STOPPED);

  }
}

bool IsStopped() { return Stopped; };

#ifdef FAST_PWM_FAN
void setPwmFrequency(uint8_t pin, int val)
{
  val &= 0x07;
  switch(digitalPinToTimer(pin))
  {

    #if defined(TCCR0A)
    case TIMER0A:
    case TIMER0B:
//         TCCR0B &= ~(_BV(CS00) | _BV(CS01) | _BV(CS02));
//         TCCR0B |= val;
         break;
    #endif

    #if defined(TCCR1A)
    case TIMER1A:
    case TIMER1B:
//         TCCR1B &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
//         TCCR1B |= val;
         break;
    #endif

    #if defined(TCCR2)
    case TIMER2:
    case TIMER2:
         TCCR2 &= ~(_BV(CS10) | _BV(CS11) | _BV(CS12));
         TCCR2 |= val;
         break;
    #endif

    #if defined(TCCR2A)
    case TIMER2A:
    case TIMER2B:
         TCCR2B &= ~(_BV(CS20) | _BV(CS21) | _BV(CS22));
         TCCR2B |= val;
         break;
    #endif

    #if defined(TCCR3A)
    case TIMER3A:
    case TIMER3B:
    case TIMER3C:
         TCCR3B &= ~(_BV(CS30) | _BV(CS31) | _BV(CS32));
         TCCR3B |= val;
         break;
    #endif

    #if defined(TCCR4A)
    case TIMER4A:
    case TIMER4B:
    case TIMER4C:
         TCCR4B &= ~(_BV(CS40) | _BV(CS41) | _BV(CS42));
         TCCR4B |= val;
         break;
   #endif

    #if defined(TCCR5A)
    case TIMER5A:
    case TIMER5B:
    case TIMER5C:
         TCCR5B &= ~(_BV(CS50) | _BV(CS51) | _BV(CS52));
         TCCR5B |= val;
         break;
   #endif

  }
}
#endif //FAST_PWM_FAN

bool setTargetedHotend(int code){
  tmp_extruder = active_extruder;
  if(code_seen('T')) {
    tmp_extruder = code_value();
    if(tmp_extruder >= EXTRUDERS) {
      SERIAL_ECHO_START;
      switch(code){
        case 104:
          SERIAL_ECHO(MSG_M104_INVALID_EXTRUDER);
          break;
        case 105:
          SERIAL_ECHO(MSG_M105_INVALID_EXTRUDER);
          break;
        case 109:
          SERIAL_ECHO(MSG_M109_INVALID_EXTRUDER);
          break;
        case 218:
          SERIAL_ECHO(MSG_M218_INVALID_EXTRUDER);
          break;
        case 221:
          SERIAL_ECHO(MSG_M221_INVALID_EXTRUDER);
          break;
      }
      SERIAL_ECHOLN(tmp_extruder);
      return true;
    }
  }
  return false;
}

float calculate_volumetric_multiplier(float diameter) {
	float area = .0;
	float radius = .0;

	radius = diameter * .5;
	if (! volumetric_enabled || radius == 0) {
		area = 1;
	}
	else {
		area = M_PI * pow(radius, 2);
	}

	return 1.0 / area;
}

void calculate_volumetric_multipliers() {
	volumetric_multiplier[0] = calculate_volumetric_multiplier(filament_size[0]);
#if EXTRUDERS > 1
	volumetric_multiplier[1] = calculate_volumetric_multiplier(filament_size[1]);
#if EXTRUDERS > 2
	volumetric_multiplier[2] = calculate_volumetric_multiplier(filament_size[2]);
#endif
#endif
}

