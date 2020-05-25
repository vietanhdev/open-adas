#include "can_bus_emitter.h"

using namespace std;

CanBusEmitter::CanBusEmitter() {
    /* open socket */
    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        perror("socket");
        return;
    }

    addr.can_family = AF_CAN;

    strcpy(ifr.ifr_name, CAN_DEVICE);
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        perror("SIOCGIFINDEX");
        return;
    }
    addr.can_ifindex = ifr.ifr_ifindex;

    if (setsockopt(s, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &enable_canfd,
                   sizeof(enable_canfd))) {
        printf("error when enabling CAN FD support\n");
        return;
    }

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return;
    }

    door_id = DEFAULT_DOOR_ID;
    signal_id = DEFAULT_SIGNAL_ID;
    speed_id = DEFAULT_SPEED_ID;

    bool seed = false;
    if (seed) {
        srand(seed);
        door_id = (rand() % 2046) + 1;
        signal_id = (rand() % 2046) + 1;
        speed_id = (rand() % 2046) + 1;
        door_pos = rand() % 9;
        signal_pos = rand() % 9;
        speed_pos = rand() % 8;
        printf("Seed: %d\n", seed);
        door_len = door_pos + 1;
        signal_len = signal_pos + 1;
        speed_len = speed_len + 2;
    } else if (model) {
        if (!strncmp(model, "bmw", 3)) {
            speed_id = MODEL_BMW_X1_SPEED_ID;
            speed_pos = MODEL_BMW_X1_SPEED_BYTE;
        } else {
            printf("Invalid model.  Valid entries are: bmw\n");
        }
    }

    if (difficulty > 0) {
        if (door_len < 8) {
            door_len += rand() % (8 - door_len);
        } else {
            door_len = 0;
        }
        if (signal_len < 8) {
            signal_len += rand() % (8 - signal_len);
        } else {
            signal_len = 0;
        }
        if (speed_len < 8) {
            speed_len += rand() % (8 - speed_len);
        } else {
            speed_len = 0;
        }
    }
}



void CanBusEmitter::send_pkt(int mtu) {
    if (write(s, &cf, mtu) != mtu) {
        perror("write");
    }
}

// Randomizes bytes in CAN packet if difficulty is hard enough
void CanBusEmitter::randomize_pkt(int start, int stop) {
    if (difficulty < 2) return;
    int i = start;
    for (; i < stop; i++) {
        if (rand() % 3 < 1) cf.data[i] = rand() % 255;
    }
}

void CanBusEmitter::send_speed() {
    if (model) {
        if (!strncmp(model, "bmw", 3)) {
            int b = ((16 * current_speed) / 256) + 208;
            int a = 16 * current_speed - ((b - 208) * 256);
            memset(&cf, 0, sizeof(cf));
            cf.can_id = speed_id;
            cf.len = speed_len;
            cf.data[speed_pos + 1] = (char)b & 0xff;
            cf.data[speed_pos] = (char)a & 0xff;
            if (current_speed == 0) {  // IDLE
                cf.data[speed_pos] = rand() % 80;
                cf.data[speed_pos + 1] = 208;
            }
            if (speed_pos) randomize_pkt(0, speed_pos);
            if (speed_len != speed_pos + 2)
                randomize_pkt(speed_pos + 2, speed_len);
            send_pkt(CAN_MTU);
        }
    } else {
        int kph = (current_speed / 0.6213751) * 100;
        memset(&cf, 0, sizeof(cf));
        cf.can_id = speed_id;
        cf.len = speed_len;
        cf.data[speed_pos + 1] = (char)kph & 0xff;
        cf.data[speed_pos] = (char)(kph >> 8) & 0xff;
        if (kph == 0) {  // IDLE
            cf.data[speed_pos] = 1;
            cf.data[speed_pos + 1] = rand() % 255 + 100;
        }
        if (speed_pos) randomize_pkt(0, speed_pos);
        if (speed_len != speed_pos + 2) randomize_pkt(speed_pos + 2, speed_len);
        send_pkt(CAN_MTU);
    }
}

void CanBusEmitter::sendSpeed(int speed) {
    int kph = (speed / 0.6213751) * 100;
    memset(&cf, 0, sizeof(cf));
    cf.can_id = speed_id;
    cf.len = speed_len;
    cf.data[speed_pos + 1] = (char)kph & 0xff;
    cf.data[speed_pos] = (char)(kph >> 8) & 0xff;
    if (kph == 0) {  // IDLE
        cf.data[speed_pos] = 1;
        cf.data[speed_pos + 1] = rand() % 255 + 100;
    }
    if (speed_pos) randomize_pkt(0, speed_pos);
    if (speed_len != speed_pos + 2) randomize_pkt(speed_pos + 2, speed_len);
    send_pkt(CAN_MTU);
}

void CanBusEmitter::send_turn_signal() {
    memset(&cf, 0, sizeof(cf));
    cf.can_id = signal_id;
    cf.len = signal_len;
    cf.data[signal_pos] = signal_state;
    if (signal_pos) randomize_pkt(0, signal_pos);
    if (signal_len != signal_pos + 1) randomize_pkt(signal_pos + 1, signal_len);
    send_pkt(CAN_MTU);
}

// // Checks if turning and activates the turn signal
// void checkTurn() {
//     if(turning < 0) {
//         signal_state ^= CAN_LEFT_SIGNAL;
//     } else if(turning > 0) {
//         signal_state ^= CAN_RIGHT_SIGNAL;
//     } else {
//         signal_state = 0;
//     }
//     send_turn_signal();
//     lastTurnSignal = currentTime;
// }