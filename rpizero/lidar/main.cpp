#include <iostream>
#include <fstream>      // std::ifstream, std::ofstream
#include "SimpleSerial.h"
#include "LIDAR.h"

using namespace std;
using namespace boost;

int main(int argc, char* argv[])
{
    try {

        SimpleSerial serial("/dev/ttyAMA0",115200);
        //std::ifstream infile("frontroom.bin",std::ifstream::binary);

        std::ofstream outfile("capture.bin",std::ofstream::binary);
        std::ofstream outcsv("capture.csv", std::ofstream::binary);

        LIDAR LiDAR;
        for (int i = 0; i < 10000; i++) {
            unsigned char c = serial.readChar();
//            char c;
  //          infile.get(c);
            cout << "c = " << c << endl;
            if (LiDAR.isHeader(c)) {
                cout <<  "found header!" << endl;
                unsigned char packet[22];
                packet[0] = c;
                for (int j = 1; j <= 21; j++) {
                    packet[j] = serial.readChar();
  //                  infile.get(c);
    //                packet[j] = c;
                }
                PACKET p = LiDAR.readPacket(packet);
                cout << "Packet: " << p.point[1].angle << " " << p.speed << endl;
                
                  // write to outfile
                outfile.write (packet, 22); // size = # elements
                

		// NOTE: removing too much stuff causes serial to starve
                
                if (p.is_valid)
                for (int i = 0; i < 4; i++) {
                       outcsv << p.point[i].angle << ","<< p.point[i].r << "," 
                              << p.point[i].quality << "," << p.point[i].invalid << ","
                              << p.point[i].strength << endl;
                }

            }
        }

/*
	TODO
	0) impl memcpy in LiDAR
	2) implement boost program options (serial port, file, etc.)
	3) implement file input or serial input switch
	4) start working algorithms (look at boost geometry)
	5) tcp input? for wifi lidar roamer bot?
	6) look at part 6 of serial stream for making SimpleSerial look like iostream: http://www.webalice.it/fede.tft/serial_port/serial_port.html
compile w/ -lboost_system
*/
        outfile.close();
        outcsv.close();

    } catch(boost::system::system_error& e)
    {
        cout<<"Error: "<<e.what()<<endl;
        return 1;
    }
    
}
