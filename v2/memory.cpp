#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif

#include <Arduino.h>
#include "memory.h"
#include <EEPROM.h>

Memory::Memory() {

}

void Memory::writeInt(int address, unsigned int number) {
  EEPROM.write(address, number >> 8);
  EEPROM.write(address + 1, number & 0xFF);
}

unsigned int Memory::readInt(int address) {
  return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
}

void Memory::Save() {
  int addr = eeprom_start_addr;
  for (int i = 0; i < cs; i++) {
    writeInt(addr + (i * intlen), config[i]);
  }

  long crc = Checksum();
  long saveCount = readLong(eeprom_count_addr);
writeLong(eeprom_crc_addr, crc);
  writeLong(eeprom_count_addr, saveCount + 1);
}

int Memory::Load() {
  long crc = Checksum();
  long _crc = readLong(eeprom_crc_addr);
  int err = 0;
  if (crc != _crc) {
    err = 1;
    return err;
  }
int addr = eeprom_start_addr;
  for (int i = 0; i < cs; i++) {
    config[i] = readInt(addr + (i * intlen));
  }

  return err;
}

long Memory::Checksum() {
  long crc = ~0L;
  for (int index = eeprom_start_addr; index < (cs*intlen)  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  return crc;
}

void Memory::writeLong(int address, long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

long Memory::readLong(int address) {
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}
