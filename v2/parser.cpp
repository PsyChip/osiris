#ifdef PROGMEM
#undef PROGMEM
#define PROGMEM __attribute__((section(".progmem.data")))
#endif

#include <Arduino.h>
#include "parser.h"

void Parser::flush() {
  command = "";
  param = "";
  buffer = "";
  lastcmd = 0;
}

Parser::Parser() {
  flush();
}

/*
    int Parser::Poll(unsigned long now);
    0: idle
    -1: unknown command
    -2: receiving
*/

int Parser::Poll(unsigned long now) {
  if ((now - lastcmd) > timeout && lastcmd > 0) {
    Serial.flush();
    flush();
  }

  if (Serial.available() > 0) {
    lastcmd = now;
    if (Receive() == true) {
      return GetCmdId();
    } else {
      return -2;
    }
  }
  return 0;
}

boolean Parser::Receive() {
  char incomingByte = (char)Serial.read();
  if (incomingByte == 13) {
    Serial.flush();
    return parse();
  } else {
    buffer += incomingByte;
  }
  return false;
}

int Parser::GetCmdId() {
  for (int i = 0; i < cmdLen; i++) {
    if (command.startsWith(Commands[i])) {
      return i + 1;
    }
  }
  return -1;
}

bool Parser::parse() {
  int fstep = 0;
  buffer.trim();
  int buflen = buffer.length();
  if (buflen < 4) {
    flush();
    return false;
  }

  for (int i = 0; i < buflen; i++) {
    if (buffer[i] == 58 && fstep == 0) {
      fstep = 1;
      continue;
    }

    if (fstep == 0) {
      command += buffer[i];
    } else {
      param += buffer[i];
    }
  }
  return true;
}

unsigned int Parser::paramInt() {
  return atol(param.c_str());
}
