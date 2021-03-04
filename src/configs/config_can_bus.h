#ifndef CONFIG_CAN_BUS_H
#define CONFIG_CAN_BUS_H


#define CAN_DEVICE "vcan0"

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
#define DEFAULT_SPEED_BYTE 3 // bytes 3,4
#define DEFAULT_SIGNAL_BYTE 0


// For now, specific models will be done as constants.  Later
// We should use a config file
#define MODEL_BMW_X1_SPEED_ID 0x1B4
#define MODEL_BMW_X1_SPEED_BYTE 0
#define MODEL_BMW_X1_RPM_ID 0x0AA
#define MODEL_BMW_X1_RPM_BYTE 4
#define MODEL_BMW_X1_HANDBRAKE_ID 0x1B4  // Not implemented yet
#define MODEL_BMW_X1_HANDBRAKE_BYTE 5
#define MODEL_BMW_X1_SPEED_BYTE 0

#endif // CONFIG_CAN_BUS_H
