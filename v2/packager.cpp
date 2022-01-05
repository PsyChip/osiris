#include <Arduino.h>
#include "Packager.h"
#include "fastRead.h"
#include "memory.h"
#include "config.h"
#include "config.cpp"

volatile long intr, _intr = 0;

///////////////////////////////////////////////////
// interrupt handler

void initRelay(int pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void rotInc() {
  if (digitalState(rot_inc) == LOW) {
    intr++;
  } else {
    intr--;
  }
}

void rotDec() {
  if (digitalState(rot_dec) == LOW) {
    intr--;
  } else {
    intr++;
  }
}

///////////////////////////////////////////////////

Button::Button(int _pin, int treshold) {
  pin = _pin;
  pinMode(pin, INPUT_PULLUP);
  digitalWrite(pin, HIGH);
  debounceDelay = treshold;
}

boolean Button::Poll(unsigned long now) {
  state = digitalRead(pin);
  boolean res = false;
  if (state != lastButtonState) {
    lastDebounceTime = now;
  }
  idle = (now - lastDebounceTime);
  if (idle >= debounceDelay) {
    if (state != buttonState) {
      buttonState = state;
      if (buttonState == LOW) {
        res = true;
      }
    }
  }

  lastButtonState = state;
  return res;
}

////////////////////////////////////////////////////////

Dolum::Dolum() {
  intr = 0;
  pinAsInputPullUp(rot_dec);
  pinAsInputPullUp(rot_inc);

  attachInterrupt(0, rotInc, RISING);
  attachInterrupt(1, rotDec, RISING);
  initRelay(out_dolum);
}

void Dolum::Start() {
  error = 0;
  reset();
  start = millis();
  digitalWrite(out_dolum, LOW);
}

void Dolum::Stop() {
  reset();
  digitalWrite(out_dolum, HIGH);
}

void Dolum::reset() {
  intr = 0;
  _intr = 0;
  rotation = 0;
  start = 0;
}

void Dolum::Poll(unsigned long now) {
  //active = (digitalRead(out_dolum) == LOW ? true : false);
  took = now - start;
  if (intr != _intr) {
    _intr = intr;
    rotation = abs(_intr);
    //  Serial.println(rotation);
  }
}
////////////////////////////////////////////////////////

Motor::Motor() {
  initRelay(out_motor);
  pinMode(in_cene_acik, INPUT_PULLUP);
  pinMode(in_cene_kapali, INPUT_PULLUP);

  digitalWrite(in_cene_acik, HIGH);
  digitalWrite(in_cene_acik, HIGH);
}

void Motor::Start() {
  digitalWrite(out_motor, LOW);
  start = millis();
  took = 0;
}

void Motor::Stop() {
  digitalWrite(out_motor, HIGH);
}

void Motor::Poll(unsigned long now) {
  _state = 0;
  if (digitalRead(in_cene_acik) == LOW) {
    _state = 2;
  }
  if (_state == 0 && digitalRead(in_cene_kapali) == HIGH) {
    _state = 3;
  }
  if (_state == 0) {
    took = now - start;
  }
  state = _state;
  if (_smem != state) {
    //   Serial.print("motor state: ");
    // Serial.println(state);
    _smem = state;
  }
}

////////////////////////////////////////////////////////

Palet::Palet() {
  initRelay(out_palet);
  pinMode(in_fotosel, INPUT_PULLUP);
  digitalWrite(in_fotosel, HIGH);
}

void Palet::Start() {
  error = 0;
  length = 0;
  sigstart = 0;
  start = millis();
  digitalWrite(out_palet, LOW);
}

void Palet::Stop() {
  error = 0;
  digitalWrite(out_palet, HIGH);
}

void Palet::Poll(unsigned long now) {
  active = (digitalRead(out_palet) == LOW ? true : false);
  signal = (digitalRead(in_fotosel) == LOW ? true : false);
  if (active == true) {
    took = now - start;
    if (took >= max_value[0]) {
      error = 1;
    }
    if (signal != _signal) {
      sigstart = now;
      _signal = signal;
    }
    if (signal == false) {
      length = now - sigstart;
    }
  } else {
    took = 0;
  }
}
////////////////////////////////////////////////////////

Tarih::Tarih() {
  initRelay(out_tarih_isi);
  initRelay(out_tarih_push);
  initRelay(out_tarih_pull);
}

void Tarih::Start() {
  reset();
  state = 1;
  digitalWrite(out_tarih_push, LOW);
}

void Tarih::reset() {
  start = millis();
  took = 0;
  Done = false;
}

void Tarih::Poll(unsigned long now) {
  took = now - start;
  switch (state) {
    case 1:
      if (took >= push) {
        digitalWrite(out_tarih_push, HIGH);
        reset();
        state = 2;
      }
      break;
    case 2:
      if (took >= tarihDelay) {
        reset();
        digitalWrite(out_tarih_pull, LOW);
        state = 3;
      }
      break;
    case 3:
      if (took >= pull) {
        digitalWrite(out_tarih_pull, HIGH);
        state = 0;
        Done = true;
      }
      break;
  }
}

//////////////////////////////////////////////
Besleme::Besleme() {
  pinMode(in_besleme, INPUT_PULLUP);
  digitalWrite(in_fotosel, HIGH);
  initRelay(out_mikser);
  initRelay(out_besleme);
  updated = millis();
}

void Besleme::Poll(unsigned long now) {
  if (_active == false) {
    return ;
  }

  int reading = digitalRead(in_besleme);
  if (reading != lastState) {
    lastdb = millis();
  }
  if ((now - lastdb) >= dbdelay) {
    if (reading != state) {
      state = reading;
      full = (reading == LOW ? true : false);
      updated = now;
    }
  }
  lastState = reading;
  idle = now - updated;
  if (full == true) {
    Stop();
  }
}

void Besleme::Start() {
  digitalWrite(out_mikser, LOW);
  digitalWrite(out_besleme, LOW);
  active = true;
}

void Besleme::Stop() {
  digitalWrite(out_besleme, HIGH);
  active = false;
}

void Besleme::Toggle(boolean on) {
  updated = millis();
  if (on == false && active == true) {
    Stop();
  }
  _active = on;
}

////////////////////////////////////////////////////////

Packager::Packager(Config conf) {
  initRelay(out_inverter);
  c = conf;
  ReadMemory();
  active = 0;
  t.push = c.tarihPush;
  t.pull = c.tarihPull;
  Launcher = new Button(btn_start, 100);
  Terminate();
}

void Packager::ReadMemory() {
  if (M.Load() > 0) {
    return;
  }
  c.boy = M.config[0];
  c.gramaj = M.config[1];
  c.tarihPush = M.config[2];
  c.tarihPull = M.config[3];
}

void Packager::SaveMemory() {
  M.config[0] = c.boy;
  M.config[1] = c.gramaj;
  M.config[2] = c.tarihPush;
  M.config[3] = c.tarihPull;
  M.Save();
}

void Packager::Start() {
  QueueStop = false;

  digitalWrite(out_inverter, LOW);
  digitalWrite(out_mikser, LOW);
  m.Start();
  state = 1;
  start = millis();
  active = true;
  Serial.println("Start");
}

void Packager::Stop() {
  Serial.println("Stopping");
  digitalWrite(out_mikser, HIGH);
  if (state > 1) {
    stopTrigger = millis();
    QueueStop = true;
    active = false;
  }
}


boolean Packager::StopCheckIn() {
  if (QueueStop == true) {
    Serial.println("Stop");
    m.Stop();
    p.Stop();
    digitalWrite(out_inverter, HIGH);
    state = 0;
    stopTrigger = 0;
    return true;
  }
  return false;
}

void Packager::Toggle() {
  if ((millis() - lastToggle) <= 1000) {
    return;
  }

  if (state == 0) {
    Start();
  } else {
    Stop();
  }
  lastToggle = millis();
}

void Packager::ButtonListener(unsigned long now) {
  if (Launcher->Poll(now) == true) {
    Toggle();
  }
}

void Packager::ConfigSync(unsigned long now) {
  if (c.changed == true) {
    t.push = c.tarihPush;
    t.pull = c.tarihPull;
    b.Toggle(c.besleme);
    c.changed = false;
    c.updated = now;
  }
  if (c.tarih == true) {
    digitalWrite(out_tarih_isi, LOW);
  } else {
    digitalWrite(out_tarih_isi, HIGH);
  }
}

void Packager::Poll(unsigned long now) {
  ButtonListener(now);
  ConfigSync(now);
  m.Poll(now);
  p.Poll(now);
  d.Poll(now);
  t.Poll(now);
  b.Poll(now);

  // Serial.println(m.jawClosed);
  //  if (_s != state) {
  //    Serial.print("state ");
  //    Serial.println(state);
  //    _s = state;
  //  }

  if (stopTrigger > 0 && (active == true && QueueStop == true) && ((now - stopTrigger) >= STOP_TIMEOUT)) {
    Serial.println("Triggered timeout");
    stopTrigger = 0;
    Terminate();
    return ;
  }

  switch (state) {
    case 0: // idle
      PinProtection();
      break;
    case 1: // cene acilis sureci
      QueueStop = false;
      if (m.state == 2 && m.took >= 250) {
        m.Stop();
        p.Start();
        state = 6;
        Serial.println("cene acik, palet akiyor");
      }
      break;
    case 2: // cene kapanis sureci
      if (m.state == 3 && m.took >= 250) {
        m.Stop();
        Serial.println("cene kapali, dolduruyor");
        s.package_count++;
        if (c.dolum == true) {
          d.Start();
          state = 3;
        }
      }
      break;
    case 3: // dolum sureci
      if (d.rotation >= c.gramaj || d.error > 0  || d.rotation >= max_value[1]) {
        d.Stop();
        if (c.tarih == true) {
          t.Start();
          state = 4;
        } else {
          m.Start();
          state = 1;
        }
      }
      break;
    case 4: // tarih baskisi
      if (t.Done == true) {
        m.Start();
        state = 1;
      }
      break;
    case 5:

      break;
    case 6:
      if (StopCheckIn() == true) {
        return ;
      }
      bool captured = false;
      switch (c.fotosel) {
        case true:
          if (p.took >= 50 && p.signal == true) {
            captured = true;
          }
          break;
        case false:
          if (p.took >= c.boy) {
            captured = true;
          }
          break;
      }
      if (captured == true || p.error > 0) {
        Serial.print("palet hatasi ");
        Serial.println(p.error);
        p.Stop();
        m.Start();
        Serial.println("Palet durdu, kapaniyor");
        state = 2;
      }
      break;
  }
}

void Packager::PinProtection() {
  digitalWrite(out_inverter, HIGH);
  digitalWrite(out_motor, HIGH);
  digitalWrite(out_palet, HIGH);
  digitalWrite(out_dolum, HIGH);
  digitalWrite(out_tarih_push, HIGH);
  digitalWrite(out_tarih_pull, HIGH);

  //digitalWrite(out_tarih_isi, HIGH);
  //digitalWrite(out_mikser, HIGH);
  //digitalWrite(out_besleme, HIGH);
}

void Packager::Terminate() {
  active = false;
  state = 0;
  digitalWrite(out_inverter, HIGH);
  digitalWrite(out_mikser, HIGH);
  digitalWrite(out_motor, HIGH);
  digitalWrite(out_palet, HIGH);
  digitalWrite(out_dolum, HIGH);
  digitalWrite(out_tarih_isi, HIGH);
  digitalWrite(out_tarih_push, HIGH);
  digitalWrite(out_tarih_pull, HIGH);
  digitalWrite(out_besleme, HIGH);
}

void Packager::ToggleOption(int id) {
  switch (id) {
    case opt_fotosel:
      c.fotosel = !c.fotosel;
      break;
    case opt_dolum:
      c.dolum = !c.dolum;
      break;
    case opt_tarih:
      c.tarih = !c.tarih;
      break;
    case opt_besleme:
      c.besleme = !c.besleme;
      break;
  }
  c.updated = millis();
  c.changed = true;
}

void Packager::SetConfig(int id, unsigned int value) {
  switch (id) {
    case conf_boy:
      c.boy = value;
      break;
    case conf_gramaj:
      c.gramaj = value;
      break;
    case conf_tarih:
      c.tarihPush = value;
      break;
    case conf_ribbon:
      c.tarihPull = value;
      break;
  }
  c.updated = millis();
  c.changed = true;
}
