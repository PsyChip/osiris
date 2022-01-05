#ifndef Parser_h
#define Parser_h

#include "Arduino.h"

#define cmdLen 9

// command format: [command:param<newline char(13)>]
// case sensitive

// ilk dördünün sırasını bozma

const String Commands[cmdLen] = {
  "boy",  "kilo",  "tarih",  "ribbon",
  "basla", "halt", "toggle", "save", "release"
};

class Parser {
  public:
    Parser();
    int Poll(unsigned long now);
    unsigned int paramInt();
    void flush();
    String buffer;
  private:
    unsigned long lastcmd = 0;
    const unsigned long timeout = 2000;
    String command;
    String param;
    int GetCmdId();
    bool Receive();
    bool parse();
};

#endif
