#define THERMAL_PROTECTION_PERIOD 60 // I hit some false positives for thermal runaway so I increased this to 1 minute
#define E0_AUTO_FAN_PIN P2_04 // This is the E1 heater pin, to which I have connected the E0 hotend fan
#define X_HOME_BUMP_MM 0 // It's not necessary to bump the home again with StallGuard
#define Y_HOME_BUMP_MM 0 // It's not necessary to bump the home again with StallGuard
#define HOMING_BACKOFF_MM { 5, 5, 0 } // I don't want the carriage/bed touching the frame if I only home X/Y
#define BLTOUCH_DELAY 200 // I find 200ms is sufficient, and I want my probing to be quicker
#define MESH_MIN_X MESH_INSET // Move the mesh in from the left edge
#define MESH_MIN_Y MESH_INSET // Move the mesh in away from the front edge
#define MESH_MAX_X X_BED_SIZE - (MESH_INSET) // Move the mesh in away from the right edge
#define MESH_MAX_Y Y_BED_SIZE - (MESH_INSET) // Move the mesh in away from the back edge
#define X_CURRENT 850 // Bumped up a bit to correspond with 1.2A max current
#define Y_CURRENT 850 // Bumped up a bit to correspond with 1.2A max current
#define Z_CURRENT 850 // Bumped up a bit to correspond with 1.2A max current
#define E0_CURRENT 940 // E3D's suggested value
#define SENSORLESS_HOMING // Use the TMC2209's StallGuard functionality for homing
#define X_STALL_SENSITIVITY 64 // This works best for me, YMMV
#define Y_STALL_SENSITIVITY 64 // This works best for me, YMMV
