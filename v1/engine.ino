#include <avr/wdt.h>

#define dolum 2                 // küçük depodan ambalaja döküm
#define palet 3                 // ambalaj çekmek için
#define motor 7                 // ana motor
<<<<<<< HEAD:osiris-v2/osiris-v2.ino
#define mikser 5                // küçük depo karıştırıcı
#define inverter 4              // hız kontrol cihazı tetikleyici

#define isiSensor A5            // isi sensoru
#define ceneSensor A4           // çene durumunu kontrol edn sensor
#define beslemeSensor A3        // besleme sensörü

=======
#define mikser 5                // küçük depo karıştırıcısıt
#define inverter 4              // hız kontrol cihazı tetikleyici

#define ceneSensor A4           // çene durumunu kontrol edn sensor
#define beslemeSensor A3        // besleme sensörü

#define isiSensor A5            // isi sensoru
>>>>>>> e8b063e855952136ace917aa8e5f62b289816999:engine/engine.ino
#define fotosel 10              // ambalaj pozisyonunu kontrol eden göz
#define power 9                 // start stop butonu
#define powerLed 11             // çalışma durumunu bildiren led
#define buzzer A0               // hata ve bilgilendirme ses çıkışı
#define afeed 12                // ambalaj besleme

#define tarih_pull 6            // pnomatik çekme vanası
#define tarih_isi A1            // tarih isi kontrol cihazi
#define tarih_pin A2            // pnomatik baskı vanası

//////////////////////////////////////////
const int cene_timeout = 9000;
const int palet_min = 250;
const int palet_max = 3000;

const int dolum_min = 25;
const int dolum_max = 1000;

const int isimin = 3000;
const long cutOutAfter = 1000; // ambalajı elle x milisaniye cektirince bir defaya mahsus ceneyi calistirir

const int tarih_max = 1000;
const int tarih_min = 10;

const int dolum_wait_delay = 25;
//////////////////////////////////////////

unsigned long tarihpinStart = 0;
unsigned long tarihpinEnd = 0;
unsigned long tarihpullStart = 0;

unsigned long paletrunsince = 0;
bool paletactive = false;

// ayarlar //////////////////////////////////////////////////////
bool fotosel_aktif = true;    // ambalajı çekerken fotosel kullan
bool dolum_aktif = true;     // deneme amaçlı,çalışırken ürün doldurma
bool tarih_aktif = true;      // ambalajı cektikten sonra tarih bas
// ayarlar //////////////////////////////////////////////////////

unsigned int tarih_pull_proc = 300;
unsigned int tarih_pull_delay = 200;

const unsigned long ping = 3000;
const int analogMin = 150;           // consider upper value is HIGH
const int emptypkgFirst = 5;         // compose x empty packages before start to fill
const int minCycle = 1;               // başlamadan önce mili x defa döndür

const int palet_max_def = 500;
const int dolum_max_def = 300;
const int tarih_max_def = 250;

const int fotosel_min = 100;          // palet en az x ms çalıştıktan sonra fotoseli oku
const int fotosel_timeout = 3000;    // palet x saniye döner ve fotosel hala çalışmazsa hatalı say
const int fotosel_error_diff = 500;  // palet bir öncekinden x ms eksik yada fazla çekerse sorunlu say

const int debounceDelay = 100;    // the debounce time; increase if the output flickers
const int toggleRate = 300;
const int fsmaxretry = 3;

int lastButtonState = -1;
unsigned long lastTrigger = 0;
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled

unsigned long bootcount = 0;
unsigned long procStart = 0;
unsigned long paletStart = 0;

unsigned long dolumStart = 0;
unsigned long dolumEnd = 0;

unsigned long prodCount = 0;
unsigned long pkgcount = 0;
unsigned long prodTotal = 0;
unsigned long lastBlip = millis();
unsigned long kesimFlip = millis();
unsigned long _paletms = 0;
unsigned int skipfill = 0;
bool app_connected = false;

int paletMax = palet_max_def;    // max package pull in ms
int dolumMax = dolum_max_def;     // helezonu x ms döndür
int tarihMax = tarih_max_def;     // tarih damgasını x milisaniye cek ve birak
int tarihPull = tarih_max_def;     // tarih seridini x ms cek ve birak

int fsretry = 0;
int fstatus = -1;
bool Fotosel = false;

bool isi = false;        // anlık ısı durumu

bool besleme = false;
bool fbesleme = false;

unsigned long isirun = 0;
unsigned long isifrun = 0;
unsigned long beslemerun = 0;

unsigned long fs_heuristic = false;
unsigned long lastfotosel = 0;

unsigned long fotoselTook = 0;
unsigned long fotoselTookEx = 0;

unsigned long dolumMs = 0;

unsigned long fm = 0;
int fs = -1;
int correction = 0;
int Status = 0;
bool active = false;
bool doldur = false;
bool kesim = false;

bool drainMode = false;
bool feedMode = false;

unsigned long kesimTTL = 0;
bool kesimx = false;
bool QueueStop = false;

unsigned long cenePing = 0;
unsigned long ceneTook = 0;
unsigned cycleCount = 0;
int spinrate = 0;
int lastStatus = 0;

void watchdogOn() {
  MCUSR = MCUSR & B11110111;
  WDTCSR = WDTCSR | B00011000;
  WDTCSR = B00100001;
  WDTCSR = WDTCSR | B01000000;
  MCUSR = MCUSR & B11110111;
}

ISR(WDT_vect) {
  // watchdog triggered
  Stop();
}

void setup() {
  pinMode(2, OUTPUT); // DOLUM
  pinMode(3, OUTPUT); // palet
  pinMode(4, OUTPUT); // inverter
  pinMode(5, OUTPUT); // mikser
  pinMode(6, OUTPUT); // buzzer
  pinMode(7, OUTPUT); // cene
  pinMode(8, OUTPUT);
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(11, OUTPUT);        // power led
  pinMode(12, INPUT_PULLUP); // ambalaj feed

  pinMode(tarih_isi, OUTPUT);
  pinMode(tarih_pin, OUTPUT);
  pinMode(tarih_pull, OUTPUT);

  digitalWrite(tarih_isi, HIGH);
  digitalWrite(tarih_pin, HIGH);
  digitalWrite(tarih_pull, HIGH);

  for (int i = 2; i < 9; i++) {
    digitalWrite(i, HIGH);
  }

  while (!Serial);
  Serial.begin(9600);
  watchdogOn();
  wdt_reset();
  bootcount++;

  Serial.print(F("$boot,"));
  Serial.print(bootcount);
  Serial.println(F(";"));

  /*8
    digitalWrite(buzzer, LOW);
    delay(500);
    digitalWrite(buzzer, HIGH);

    digitalWrite(tarih_pin, LOW);
    delay(tarihMax);
    digitalWrite(tarih_pin, HIGH);
    delay(100);
    digitalWrite(tarih_pull, LOW);
    delay(400);
    digitalWrite(tarih_pull, HIGH);
  */
}

int bliprate = 100;
int blipduration = 100;
unsigned long blipstart = 0;
bool blipactive = false;
unsigned long lastAppInfo = 0;
int appInfoInterval = 500;

void loop() {
  BLIP();
  appInfo(false);
  checkSensors();

  if (drainMode == false) {
    if (besleme == true && dolum_aktif == true) {
      digitalWrite(mikser, LOW);
    } else {
      if (active == false) {
        digitalWrite(mikser, HIGH);
      }
    }
  }

  if (tarih_aktif == true) {
    digitalWrite(tarih_isi, LOW);
  } else {
    digitalWrite(tarih_isi, HIGH);
  }

  if (ParserCheckIn() == true) {
    return ;
  }

  if (active == false) {
    if (digitalRead(afeed) == LOW) {
      feed(true);
      if (paletactive == false) {
        paletrunsince = millis();
        paletactive = true;
      }
    } else {
      if (digitalRead(palet) == LOW) {
        feed(false);
        if (paletactive == true) {
          paletactive = false;
          if ((millis() - paletrunsince) >= cutOutAfter) {
            cutOff();
          }
        }
      }
    }
  }

  if (checkButton(power) == true) {
    toggle();
  }

  if (active == true) {
    cenePozisyon();

    if ((digitalRead(motor) == LOW) && (millis() - kesimFlip) >= cene_timeout) {
      Error(3);
      Stop();
      return ;
    }

    if (Status != lastStatus) {
      lastStatus = Status;
      //  Serial.println(Status);
    }
  }

  if (active == true && cycleCount >= minCycle) {
    unsigned long now = millis();

    if (pkgcount <= emptypkgFirst) {
      doldur = false;
      correction = 1;
    } else {
      if (QueueStop == false) {
        digitalWrite(mikser, LOW);
        digitalWrite(powerLed, LOW);
      }
      correction = 2;
      doldur = true;
    }
    
    switch (Status) {
      case 1: // motor hareket halinde
        if (((now - kesimTTL) >= 1000)) {
          wdt_reset();
          break;
        }

        if (QueueStop == true && kesim == false) {
          QueueStop = false;
          Stop();
          break;
        }

        if (kesimx != kesim) {
          kesimFlip = millis();
          kesimx = kesim;
          switch (kesim) {
            case true:
              pkgcount++;
              if (doldur == true && dolum_aktif == true) {
                digitalWrite(motor, HIGH);
                dolumBegin();
              }
              break;
            case false:
              digitalWrite(tarih_pin, HIGH);
              digitalWrite(motor, HIGH);
              paletBegin();
              break;
          }
        }


        if (kesim == true) {
          if (kesimx != kesim) {
            kesimx = kesim;
          }
        }
        break;

      case 2: // palet hareket halinde
        _paletms = (millis() - paletStart);
        if (QueueStop == true) {
          QueueStop = false;
          Status = 1;
          fotoselTookEx = 0;
          fotoselTook = 0;
          lastfotosel = 0;
          Fotosel = true;
          Stop();
          break;
        }
        if (fotosel_aktif == true) {
          fm = millis();
          fs = digitalRead(fotosel);
          if (fs == LOW && fstatus != fs && _paletms >= fotosel_min) {
            if (fotoselTook > 0) {
              fotoselTookEx = fotoselTook;
            }
            if (lastfotosel > 0) {
              fotoselTook = fm - lastfotosel;
            }
            lastfotosel = fm;
            Fotosel = true;
          }
          fstatus = fs;
          bool ferr = false;
          /*
            int fotodiff = fotoselTook - fotoselTookEx;
            if ((abs(fotodiff) >= fotosel_error_diff) && fotoselTookEx > 0 && fotoselTook > 0 && Fotosel == true) {
            paletEnd();
            Fotosel = false;
            blipFor(250, 500);
            skipfill = 2;
            ferr = true;
            }

          */
          if (((_paletms >= fotosel_timeout) || Fotosel == true) && ferr == false) {
            paletEnd();

            if (Fotosel == false) {
              fsretry++;
              fotoselTookEx = 0;
              fotoselTook = 0;
              blipFor(250, 500);
              skipfill = 2;
              ferr = true;
              if (fsretry > fsmaxretry) {
                QueueStop = true;
                Error(2);
              }
            } else {
              fsretry = 0;
            }
            Fotosel = false;
          }

        } else {
          if (_paletms >= paletMax) {
            paletEnd();
          }
        }
        break;

      case 3: // 2 aşamalı tarih baskısı
        if (( millis() - tarihpinStart) >= tarihMax) {
          digitalWrite(tarih_pin, HIGH);
          tarihpinEnd =  millis();
          Status = 4;
        }
        break;
      case 4:
        if ((millis() - tarihpinEnd) >= tarih_pull_delay) {
          tarihpinEnd = 0;
          digitalWrite(tarih_pull, LOW);
          tarihpullStart =  millis();
          Status = 5;
        }
        break;
      case 5:
        if (tarihpullStart > 0 && ( millis() - tarihpullStart) >= tarih_pull_proc) {
          digitalWrite(tarih_pull, HIGH);
          digitalWrite(inverter, LOW);
          tarihpullStart = 0;
          motorBegin();
        }
        break;
      case 6: // dolum
        if ((millis() - dolumStart) >= dolumMax) {
          digitalWrite(dolum, HIGH);
          motorBegin();
        }
        break;
    }
  }
  wdt_reset();
}
