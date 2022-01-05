#ifndef Memory_h
#define Memory_h
#include "Arduino.h"

#define eeprom_count_addr 0
#define eeprom_crc_addr 4
#define eeprom_start_addr 8

#define cs 4                // memory size

class Memory {
  public:
    Memory();
    void Save();
    int Load();
    int config[cs] = {0, 0, 0, 0};
  private:
    long Checksum();
    void writeInt(int address, unsigned int number);
    unsigned int readInt(int address);
    void writeLong(int address, long value);
    long readLong(int address);
    int intlen = 2;
    int longlen = 4;
    const unsigned long crc_table[16] = {
      0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
      0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
      0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
      0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
    };
};

#endif
