#ifndef CAN_READER_H
#define CAN_READER_H

#include <getopt.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <atomic>
#include <iostream>

#include "sensors/libs/can_lib/can_lib.h"
#include "configs/config_can_bus.h"

class CANReader {
   private:
    const int canfd_on = 1;
    int debug = 0;
    int randomize = 0;
    int signal_pos = DEFAULT_SIGNAL_BYTE;
    int speed_pos = DEFAULT_SPEED_BYTE;
    std::atomic<int> current_speed = {0};
    int turn_status[2];
    char *model = NULL;
    char data_file[256];
    int opt;
    int can;
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct canfd_frame frame;
    struct iovec iov;
    struct msghdr msg;
    struct cmsghdr *cmsg;
    struct timeval tv, timeout_config = {0, 0};
    struct stat dirstat;
    fd_set rdfs;
    char
        ctrlmsg[CMSG_SPACE(sizeof(struct timeval)) + CMSG_SPACE(sizeof(__u32))];
    int running = 1;
    int nbytes, maxdlen;
    int ret;
    int seed = 0;
    int signal_id, speed_id;

    /* Parses CAN frame and updates turn signal status */
    void update_signal_status(struct canfd_frame *cf, int maxdlen);

    /* Parses CAN fram and updates current_speed */
    void update_speed_status(struct canfd_frame *cf, int maxdlen);

   public:
    CANReader();
    int getSpeed();
    bool getLeftTurnSignal();
    bool getRightTurnSignal();
    void readCANSignal();
};

#endif  // CAN_READER_H
