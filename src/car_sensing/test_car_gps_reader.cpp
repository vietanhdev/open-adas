#include <iostream>
#include <fstream>
#include "car_gps_reader.h"

using namespace std;
   
int main(int argc, char const *argv[]) { 

    CarGPSReader reader;

    while (true) {
        int ret = reader.updateProps();

        if (ret != 0) {
            cout << "Error on reading" << endl;
            reader.printError();
        }
        cout << "Long: " << reader.getLongitude() << " N, " << reader.getLatitude() << " E" << endl;
        cout << "Speed: " << reader.getCarSpeed() << " km/h" << endl;
    }

    return 0;
} 