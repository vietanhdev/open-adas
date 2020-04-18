
#include <stdio.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <iostream>
#include <fstream>
#include <iomanip>

#include <nmeaparse/nmea.h>

using namespace nmea;
using namespace std;


class CarPropReader {

    private:

        // Socket setting to read GPS data from
        int socket_port;
        std::string socket_server;


    CarPropReader() {
        this->socket_port = 50000;
        this->socket_server = "192.168.1.244"; // TODO: Fix this

        // Create a GPS service that will keep track of the fix data.
        NMEAParser parser;
        GPSService gps(parser);
        parser.log = false;
    }



};