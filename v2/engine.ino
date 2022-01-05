#include <Wire.h>
#include <avr/power.h>

#include "packager.h"
#include "parser.h"

Config conf = {
  true,   // fotosel
  true,  // dolum
  true,   // tarih
  true,   // besleme
  500,    // boy
  1750,   // gramaj
  250,    // tarih push
  150,    // tarih pull
};

Packager *p;
Parser *r;
const char comma = ',';
unsigned long lastfeedback = 0;
unsigned int input_mem = 0;

/*
 * 
 * 
// feed function
unsigned long feedstart = 0;
const int feedcutout = 1000;
const int cutspeed = 2200;

void paperPoll(unsigned long now) {
  if (p->active == true) {
    return ;
  }
  if (digitalRead(btn_feed) == LOW) {
    if (feedstart == 0) {
      feedstart = now;
      digitalWrite(out_inverter, LOW);
    }
    digitalWrite(out_palet, LOW);
  } else {
    if (feedstart == 0) {
      return ;
    }
    if (feedstart >= feedcutout) {
      digitalWrite(out_palet, HIGH);
      digitalWrite(out_motor, LOW);
      delay(cutspeed);
      digitalWrite(out_motor, HIGH);
      digitalWrite(out_inverter, HIGH);
    } else {
      digitalWrite(out_palet, HIGH);
      delay(10);
      digitalWrite(out_inverter, HIGH);
    }
    feedstart = 0;
  }
}
*/
// feed end

void setup() {
  delay(50);
  while (!Serial);
  Serial.begin(9600);
  InitLowPower();
  WatchdogInit();
  r = new Parser();
  p = new Packager(conf);
  ack(0, 0);
}

void ack(int i, int val) {
  Serial.print(F("$ack,"));
  Serial.print(i);
  Serial.print(comma);
  Serial.print(val);
  Serial.println(F(";"));
}

int ReadInputState() {
  boolean bin[8] = {
    !digitalRead(in_fotosel),
    !digitalRead(in_cene_kapali),
    !digitalRead(in_besleme),
    !digitalRead(btn_start),
    0,
    0,
    0,
    0
  };
  return BinToDec(bin);
}

int BinToDec(boolean Bin[]) {
  int ReturnInt = 0;
  for (int i = 0; i < 8; i++) {
    if (Bin[7 - i]) {
      ReturnInt += 1 << i;
    }
  }
  return ReturnInt;
}

void feedback() {
  Serial.print(F("$inf,"));
  boolean bin[8] = {
    p->active,
    p->c.besleme,
    p->c.fotosel,
    p->c.dolum,
    p->c.tarih,
    !digitalRead(out_besleme),
    !digitalRead(out_dolum),
    0
  };

  Serial.print(p->c.gramaj);   // gramaj ayari
  Serial.print(comma);
  Serial.print(p->c.boy); // boy ayari
  Serial.print(comma);
  Serial.print(p->c.tarihPush); // tarih itme
  Serial.print(comma);
  Serial.print(p->c.tarihPull); // tarih cekme
  Serial.print(comma);
  Serial.print(p->s.package_count); // toplam paket
  Serial.print(comma);
  Serial.print(BinToDec(bin));
  Serial.print(comma);
  Serial.print(ReadInputState());
  Serial.println(F(";"));
}

void feedPoll(unsigned long now) {
  if ((now - lastfeedback) >= 1000) {
    feedback();
    lastfeedback = now;
  }
}

bool parseCommand(int c) {
  if (c != 0) {
    switch (c) {
      case -2:
        return true;
        // still receiving
        break;
      case -1:
        Serial.print("$unk,");
        Serial.print(r->buffer);
        Serial.print(comma);
        Serial.println(F(";"));
        // unknown command
        break;
      case 1:
      case 2:
      case 3:
      case 4:
        p->SetConfig(c, r->paramInt());
        ack(c, r->paramInt());
        break;
      case 5:
        p->Toggle();
        break;
      case 6:
        p->Terminate();
        break;
      case 7:
        p->ToggleOption(r->paramInt());
        feedback();
        break;
      case 8:
        p->SaveMemory();
        Serial.println("Save Success");
        break;
      case 9:
        // hazne bosalt
        break;
      default:
        Serial.println("+una:"); // unassigned command
    };
    r->flush();
    return true;
  }
  return false;
}

void loop() {
  unsigned long now = millis();
  int c = r->Poll(now);
  if (parseCommand(c) == true) {
    return ;
  }
  int S = ReadInputState();
  if(S!=input_mem) {
    input_mem = S;
    ack(255,S);
  }
  //paperPoll(now);
  feedPoll(now);
  p->Poll(now);
}
