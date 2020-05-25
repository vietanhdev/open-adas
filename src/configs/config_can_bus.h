#if !defined(CONFIG_CAN_BUS_H)
#define CONFIG_CAN_BUS_H


#define CAN_DEVICE "vcan0"

#ifndef DATA_DIR
#define DATA_DIR "./data/"
#endif
#define DEFAULT_CAN_TRAFFIC DATA_DIR "sample-can.log"

#define DEFAULT_DIFFICULTY 1
// 0 = No randomization added to the packets other than location and ID
// 1 = Add NULL padding
// 2 = Randomize unused bytes
#define DEFAULT_DOOR_ID 411
#define DEFAULT_DOOR_POS 2
#define DEFAULT_SIGNAL_ID 392
#define DEFAULT_SIGNAL_POS 0
#define DEFAULT_SPEED_ID 580
#define DEFAULT_SPEED_POS 3
#define CAN_DOOR1_LOCK 1
#define CAN_DOOR2_LOCK 2 
#define CAN_DOOR3_LOCK 4
#define CAN_DOOR4_LOCK 8
#define CAN_LEFT_SIGNAL 1
#define CAN_RIGHT_SIGNAL 2
#define ON 1
#define OFF 0
#define DOOR_LOCKED 0
#define DOOR_UNLOCKED 1
#define SCREEN_WIDTH 835
#define SCREEN_HEIGHT 608
#define JOY_UNKNOWN -1
#define BUTTON_LOCK 4
#define PS3_BUTTON_LOCK 10
#define BUTTON_UNLOCK 5
#define PS3_BUTTON_UNLOCK 11
#define BUTTON_A 0
#define PS3_BUTTON_A 14
#define BUTTON_B 1
#define PS3_BUTTON_B 13
#define BUTTON_X 2
#define PS3_BUTTON_X 15 
#define BUTTON_Y 3
#define PS3_BUTTON_Y 12
#define BUTTON_START 7
#define PS3_BUTTON_START 3
#define AXIS_LEFT_V 0
#define PS3_AXIS_LEFT_V 0
#define AXIS_LEFT_H 1
#define PS3_AXIS_LEFT_H 1
#define AXIS_L2 2
#define PS3_AXIS_L2 12
#define AXIS_RIGHT_H 3
#define PS3_AXIS_RIGHT_H 3
#define AXIS_RIGHT_V 4
#define PS3_AXIS_RIGHT_V 2
#define AXIS_R2 5
#define PS3_AXIS_R2 13
#define PS3_X_ROT 4
#define PS3_Y_ROT 5
#define PS3_Z_ROT 6 // The rotations are just guessed
#define MAX_SPEED 90.0 // Limiter 260.0 is full guage speed
#define ACCEL_RATE 8.0 // 0-MAX_SPEED in seconds
#define USB_CONTROLLER 0
#define PS3_CONTROLLER 1

// For now, specific models will be done as constants.  Later
// We should use a config file
#define MODEL_BMW_X1_SPEED_ID 0x1B4
#define MODEL_BMW_X1_SPEED_BYTE 0
#define MODEL_BMW_X1_RPM_ID 0x0AA
#define MODEL_BMW_X1_RPM_BYTE 4
#define MODEL_BMW_X1_HANDBRAKE_ID 0x1B4  // Not implemented yet
#define MODEL_BMW_X1_HANDBRAKE_BYTE 5

#endif // CONFIG_CAN_BUS_H
