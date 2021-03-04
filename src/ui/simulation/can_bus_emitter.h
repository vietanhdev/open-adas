#ifndef CAN_BUS_EMITTER_H
#define CAN_BUS_EMITTER_H


#include <iostream>
#include <getopt.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#include "configs/config_can_bus.h"

class CanBusEmitter {
   private:
    int s;  // socket
    struct canfd_frame cf;
    struct ifreq ifr;
    int door_pos = DEFAULT_DOOR_POS;
    int signal_pos = DEFAULT_SIGNAL_POS;
    int speed_pos = DEFAULT_SPEED_POS;
    int door_len = DEFAULT_DOOR_POS + 1;
    int signal_len = DEFAULT_DOOR_POS + 1;
    int speed_len = DEFAULT_SPEED_POS + 2;
    int difficulty = DEFAULT_DIFFICULTY;
    char *model = NULL;
    int lock_enabled = 0;
    int unlock_enabled = 0;
    char door_state = 0xf;
    char signal_state = 0;
    int throttle = 0;
    float current_speed = 0;
    int turning = 0;
    int door_id, signal_id, speed_id;
    int currentTime;
    int lastAccel = 0;
    int lastTurnSignal = 0;

    int seed = 0;
    int debug = 0;

    int play_id;
    int kk = 0;
    char data_file[256];

    int opt;
    struct sockaddr_can addr;
    struct canfd_frame frame;
    int running = 1;
    int enable_canfd = 1;
    int play_traffic = 1;
    struct stat st;

   public:
    CanBusEmitter();

   private:
    char *get_data(char *fname);
    void send_pkt(int mtu);

    // Randomizes bytes in CAN packet if difficulty is hard enough
    void randomize_pkt(int start, int stop);

    void send_speed();
    void send_turn_signal();

   public:
    void sendSpeed(int speed);
    void sendTurnSignal(bool turning_left, bool turning_right);
};

#endif  // CAN_BUS_SENDER_H
