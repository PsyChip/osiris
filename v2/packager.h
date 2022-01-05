#ifndef Packager_h
#define Packager_h

#include "Arduino.h"
#include "config.h"
#include "memory.h"

struct Stats {
  unsigned long package_count = 0;
};

struct Config {
  boolean fotosel;
  boolean dolum;
  boolean tarih;
  boolean besleme;
  unsigned int boy;
  unsigned int gramaj;
  unsigned int tarihPush;
  unsigned int tarihPull;
  boolean changed;
  unsigned long updated;
};

struct Error {
  int module;
  int code;
  int state;
  int warnings;
  unsigned long time;
};

class Button {
  public:
    Button(int _pin, int treshold);
    boolean Poll(unsigned long now);
    int pin;
    int state;
    unsigned long idle;
  private:
    int buttonState;
    int lastButtonState = LOW;
    unsigned long lastDebounceTime = 0;
    unsigned long debounceDelay = DEBOUNCE_DELAY_BUTTON;
};

class Besleme {
  public:
    Besleme();
    void Toggle(boolean on);
    boolean active = false;
    boolean full = false;
    void Poll(unsigned long now);
  private:
    unsigned long updated;
    int state;
    int lastState = LOW;
    unsigned long lastdb = 0;
    unsigned long dbdelay = (DEBOUNCE_DELAY_BUTTON * 2);
    unsigned long idle;
    void Start();
    void Stop();
    boolean _active = false;
};

class Motor {
  public:
    Motor();
    void Stop();
    void Start();
    void Poll(unsigned long now);
    unsigned int state = 0; // 0: unknown, 1: rolling, 2:open, 3: closed
    unsigned long took;
  private:
    unsigned int _state = 0;
    unsigned int _smem = 0;
    unsigned long start;
};

class Dolum {
  public:
    Dolum();
    void Start();
    void Stop();
    void Poll(unsigned long now);
    long rotation;
    boolean active = false;
    unsigned long took;
    int error = 0;
  private:
    unsigned long start;
    void reset();
};

class Palet {
  public:
    Palet();
    void Start();
    void Stop();
    void Poll(unsigned long now);
    boolean active = false;
    unsigned long took;
    boolean signal = false;
    unsigned long length;
    unsigned int error;
  private:
    boolean _signal = false;
    unsigned long start;
    unsigned long sigstart;
};

class Tarih {
  public:
    Tarih();
    void Start();
    void Poll(unsigned long now);
    boolean Done;
    int push;
    int pull;
  private:
    unsigned long took;
    unsigned long start;
    int state;
    void reset();
    const int tarihDelay = TARIH_DELAY;      // delay between push and pull
};

class Packager {
  public:
    Stats s;
    Error e;
    Packager(Config conf);
    Dolum d;
    Besleme b;
    Config c;
    
    int state;
    bool active;
    void Toggle();
    void Start();
    void Stop();
    void Poll(unsigned long now);
    void Terminate();
    void SetConfig(int id, unsigned int value);
    void ToggleOption(int id);
    void ReadMemory();
    void SaveMemory();
  private:
    unsigned long lastToggle = 0;
    unsigned long start;
    unsigned long stopTrigger = 0;
    boolean QueueStop;
    boolean StopCheckIn();
    void PinProtection();
    void FinalizeCycle();
    void ResetStats();
    void ConfigSync(unsigned long now);
    void ButtonListener(unsigned long now);
    Memory M;

    int _s = 0;
    Button *Launcher;    
    
    
    Motor m;
    Palet p;
    Tarih t;
};

#endif
