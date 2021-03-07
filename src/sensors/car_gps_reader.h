#ifndef CAR_PROP_READER_H
#define CAR_PROP_READER_H

#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <iostream>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <nmeaparse/nmea.h>

using namespace nmea;
using namespace std;


enum SignalStatus {
    kSignalNormal = 0,
    kSignalNotConnected = 1,
    kSignalsocketCreationError = -1,
    kSignalInvalidAddress = -2,
    kSignalConnectionFailed = -3
};

class CarGPSReader {

  private:

    // Socket setting to read GPS data from socket server
    int socket_port;
    std::string socket_server;
    int sock = 0, valread;
    struct sockaddr_in serv_addr; 
    char buffer[1024] = {0}; 

    // GPS parser
    std::unique_ptr<NMEAParser> nmea_parser;
    std::unique_ptr<GPSService> gps;


  public:
    // Status of car
    // 0: Connected to server
    // 1: Not connected to server; 
    // -1: Socket creation error
    // -2: Invalid address/ Address not supported
    // -3: Connection failed
    int signal_status = kSignalNotConnected;
    

    std::mutex car_prop_mutex;
    float car_speed = 0; // km/h
    float longitude = 0;
    float latitude = 0;

  public:
    CarGPSReader();

    int printError();

    // Update car properties
    // Return 0: success
    // Return 1: fail
    int updateProps();
    
    float getLongitude();
    float getLatitude();
    float getCarSpeed();
    float getSignalStatus();
    void setSignalStatus(SignalStatus status) ;

  private:

    int init_socket_conn();

};

#endif