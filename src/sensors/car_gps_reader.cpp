#include "car_gps_reader.h"

CarGPSReader::CarGPSReader() {
    this->socket_port = 50000;
    this->socket_server = "192.168.1.244";  // TODO: Use dynamic address

    // Create a GPS service that will keep track of the fix data.
    nmea_parser = std::make_unique<NMEAParser>();
    gps = std::make_unique<GPSService>(*nmea_parser.get());
    nmea_parser->log = false;
}

int CarGPSReader::printError() {
    switch (signal_status) {
        case kSignalNormal:
            cout << "Socket: No error" << endl;
            break;
        case kSignalNotConnected:
            cout << "Socket: Not connected" << endl;
            break;
        case kSignalsocketCreationError:
            cout << "Socket: Socket creation error" << endl;
            break;
        case kSignalInvalidAddress:
            cout << "Socket: Invalid address/ Address not supported" << endl;
            break;
        case kSignalConnectionFailed:
            cout << "Socket: Connection failed" << endl;
            break;
    }
    return signal_status;
}

// Update car properties
// Return 0: success
// Return 1: fail
int CarGPSReader::updateProps() {
    if (getSignalStatus() == kSignalNotConnected) {
        init_socket_conn();
    }

    valread = read(sock, buffer, 1024);
    if (valread <= 0) {
        // cerr << "Recreate connection" << endl;
        init_socket_conn();
        valread = read(sock, buffer, 1024);
    }

    if (valread <= 0) {
        return 1;
    }

    signal_status = 0;  // Connected

    buffer[valread] = '\n';

    // From a buffer in memory...
    nmea_parser->readBuffer((uint8_t *)buffer, sizeof(buffer));

    std::lock_guard<std::mutex> lk(car_prop_mutex);
    signal_status = kSignalNormal;
    longitude = gps->fix.longitude;
    latitude = gps->fix.latitude;
    car_speed = gps->fix.speed;

    return 0;
}

float CarGPSReader::getLongitude() {
    std::lock_guard<std::mutex> lk(car_prop_mutex);
    return longitude;
}

float CarGPSReader::getLatitude() {
    std::lock_guard<std::mutex> lk(car_prop_mutex);
    return latitude;
}

float CarGPSReader::getCarSpeed() {
    std::lock_guard<std::mutex> lk(car_prop_mutex);
    return car_speed;
}

float CarGPSReader::getSignalStatus() {
    std::lock_guard<std::mutex> lk(car_prop_mutex);
    return signal_status;
}

void CarGPSReader::setSignalStatus(SignalStatus status) {
    std::lock_guard<std::mutex> lk(car_prop_mutex);
    signal_status = status;
}

int CarGPSReader::init_socket_conn() {
    if (sock != 0) {
        close(sock);
        sock = 0;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        setSignalStatus(kSignalsocketCreationError);
        // cerr << "\n Socket creation error \n" << endl;
        return kSignalsocketCreationError;
    }

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(socket_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, socket_server.c_str(), &serv_addr.sin_addr) <= 0) {
        setSignalStatus(kSignalInvalidAddress);
        // cerr << "\nInvalid address/ Address not supported \n" << endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        setSignalStatus(kSignalConnectionFailed);
        // cerr << "\nConnection Failed \n" << endl;
        return -1;
    }

    return 0;
}