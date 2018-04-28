#pragma once

struct POINT {
    int r;
    int angle;
    int quality;
    bool invalid;
    bool strength;
};

struct PACKET {
    POINT point[4];
    int number;
    int speed;
    bool is_valid;
};

struct SWEEP {
    PACKET data[90];
};

const unsigned char INVALID_DATA_FLAG = (1 << 7);   // Mask for byte 1 of each data quad "Invalid data"
const unsigned char STRENGTH_WARNING_FLAG = (1 << 6);  // Mask for byte 1 of each data quat "Strength Warning"
const unsigned char BAD_DATA_MASK = (INVALID_DATA_FLAG | STRENGTH_WARNING_FLAG);

class LIDAR
{
    /*
    *   LIDAR interprets an incoming stream of bytes and provides position/quality/validity data
    */
public:
    LIDAR()
    {
    }

    // Returns true if header
    bool isHeader(unsigned char c)
    {
        return c==0xFA;
    }

    // Returns packetNumber
    int packetNumber(unsigned char inByte)
    {
        return inByte - 0xA0;
    }

    // Returns LiDAR speed
    double speed(unsigned char lowByte, unsigned char highByte)
    {
        return (highByte << 8) | lowByte;
    }

    // Returns data point
    POINT point(unsigned char input[4])
    {
        POINT p;
        p.r = input[0] | (( input[1] & 0x3f) << 8); // # distance is coded on 13 bits ? 14 bits ? THIS MATCHES XV_Lidar_Controller.ino
        p.invalid  = input[1] & INVALID_DATA_FLAG;
        p.strength = input[1] & STRENGTH_WARNING_FLAG;
        p.quality  = input[2] | (input[3] << 8);
        return p;
    }

    bool checksum(unsigned char lowByte, unsigned char highByte, unsigned char packet[20])
    {
      unsigned long checksum = 0;
      for (int i = 0; i < 10; i++) {
        checksum = (checksum << 1) + (packet[2 * i] + (packet[2 * i + 1] << 8));
      }
      checksum = ((checksum & 0x7FFF) + (checksum >> 15)) & 0x7FFF;

      if ( (lowByte + (highByte << 8)) == checksum )
      {
        return true;
      } else {
        return false;
      }
    }

    // read an entire packet
    PACKET readPacket( unsigned char packet[22])
    {
        PACKET pkt;
        pkt.is_valid = isHeader(packet[0]);
        if(!pkt.is_valid)
            return pkt;
        pkt.number = packetNumber(packet[1]);
        pkt.speed  = speed(packet[2], packet[3]) / 64.;

    	// TODO replace w/memcpy (?) Needs to be light for Arduino
        for (int i = 0; i < 4; i++)
        {
            unsigned char data[4];
            for (int j = 0; j < 4; j++)
                data[j] = packet[4+i*4+j];
            POINT p = point(data);
            p.angle = 4*pkt.number + i;
            pkt.point[i] = p;
        }

        pkt.is_valid = checksum(packet[20], packet[21], packet);
        return pkt;
    }
};
