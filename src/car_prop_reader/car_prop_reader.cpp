// Client side C/C++ program to demonstrate Socket programming 
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

#define PORT 50000 
   
int main(int argc, char const *argv[]) 
{ 

    // Fill with your NMEA bytes... make sure it ends with \n
	char bytestream[] = "\n";

	// Create a GPS service that will keep track of the fix data.
	NMEAParser parser;
	GPSService gps(parser);
	parser.log = false;
	
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
       
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "192.168.1.244", &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 
   
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    while (true) {
        valread = read( sock , buffer, 1024);

        buffer[valread] = '\n';

        // From a buffer in memory...
	    parser.readBuffer((uint8_t*)buffer, sizeof(buffer));


        float lon = gps.fix.longitude;
        float lat = gps.fix.latitude;
        float speed = gps.fix.speed;

        // cout << buffer << endl;
        cout << "Long: " << lon << " N, " << lat << " E" << endl;
        cout << "Speed: " << speed << " km/h" << endl;

    }
    
    return 0; 
} 