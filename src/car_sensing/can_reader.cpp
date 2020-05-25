#include "can_reader.h"

using namespace std;

CANReader::CANReader() {

    speed_id = DEFAULT_SPEED_ID;
    signal_id = DEFAULT_SIGNAL_ID;

    // Create a new raw CAN socket
    can = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can < 0) cerr << "Couldn't create raw socket" << endl;

    addr.can_family = AF_CAN;
    memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    strncpy(ifr.ifr_name, CAN_DEVICE, strlen(CAN_DEVICE));
    printf("Using CAN interface %s\n", ifr.ifr_name);
    if (ioctl(can, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        exit(1);
    }
    addr.can_ifindex = ifr.ifr_ifindex;
    // CAN FD Mode
    setsockopt(can, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canfd_on,
               sizeof(canfd_on));

    iov.iov_base = &frame;
    iov.iov_len = sizeof(frame);
    msg.msg_name = &addr;
    msg.msg_namelen = sizeof(addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &ctrlmsg;
    msg.msg_controllen = sizeof(ctrlmsg);
    msg.msg_flags = 0;

    if (bind(can, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }

    if (model) {
        if (!strncmp(model, "bmw", 3)) {
            speed_id = MODEL_BMW_X1_SPEED_ID;
            speed_pos = MODEL_BMW_X1_SPEED_BYTE;
        } else {
            printf("Unknown model.  Acceptable models: bmw\n");
            exit(3);
        }
    }
    
}

void CANReader::readCANSignal() {
    nbytes = recvmsg(can, &msg, 0);
    if (nbytes < 0) {
        perror("read");
        cout << "Error reading from CAN" << endl;
        return;
    }
    if ((size_t)nbytes == CAN_MTU)
        maxdlen = CAN_MAX_DLEN;
    else if ((size_t)nbytes == CANFD_MTU)
        maxdlen = CANFD_MAX_DLEN;
    else {
        fprintf(stderr, "read: incomplete CAN frame\n");
        return;
    }
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg && (cmsg->cmsg_level == SOL_SOCKET);
         cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        if (cmsg->cmsg_type == SO_TIMESTAMP)
            tv = *(struct timeval *)CMSG_DATA(cmsg);
        else if (cmsg->cmsg_type == SO_RXQ_OVFL)
            // dropcnt[i] = *(__u32 *)CMSG_DATA(cmsg);
            fprintf(stderr, "Dropped packet\n");
    }
    //      if(debug) fprint_canframe(stdout, &frame, "\n", 0, maxdlen);
    if (frame.can_id == signal_id) update_signal_status(&frame, maxdlen);
    if (frame.can_id == speed_id) update_speed_status(&frame, maxdlen);
}

/* Parses CAN frame and updates turn signal status */
void CANReader::update_signal_status(struct canfd_frame *cf, int maxdlen) {
    int len = (cf->len > maxdlen) ? maxdlen : cf->len;
    if (len < signal_pos) return;
    if (cf->data[signal_pos] & CAN_LEFT_SIGNAL) {
        turn_status[0] = ON;
    } else {
        turn_status[0] = OFF;
    }
    if (cf->data[signal_pos] & CAN_RIGHT_SIGNAL) {
        turn_status[1] = ON;
    } else {
        turn_status[1] = OFF;
    }
    // update_turn_signals();
}

/* Parses CAN fram and updates current_speed */
void CANReader::update_speed_status(struct canfd_frame *cf, int maxdlen) {
    int len = (cf->len > maxdlen) ? maxdlen : cf->len;
    if (len < speed_pos + 1) return;
    if (model) {
        if (!strncmp(model, "bmw", 3)) {
            current_speed = (((cf->data[speed_pos + 1] - 208) * 256) +
                             cf->data[speed_pos]) /
                            16;
        }
    } else {
        int speed = cf->data[speed_pos] << 8;
        speed += cf->data[speed_pos + 1];
        speed = speed / 100;                // speed in kilometers
        current_speed = speed * 0.6213751;  // mph
    }
}

int CANReader::getSpeed() {
    return current_speed;
}
