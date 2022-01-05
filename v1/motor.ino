
void cenePozisyon() {
  int i = analogRead(ceneSensor);
  if (i >= analogMin) {
    ceneTook = millis() - cenePing;
    if (kesim == false) {
      cenePing = millis();
      cycleCount++;
      kesim = true;
      kesimTTL = millis();
      //     ceneStats(0);
    }
  }

  if (i < 50) {
    if (kesim == true) {
      //    ceneStats(1);
    }
    kesim = false;
    kesimTTL = millis();
  }
}


void appInfo(bool now) {
  if (app_connected == false) {
    //return;
  }
  if (now == true) {
    lastAppInfo = millis() - appInfoInterval - 100;
  }
  if ((millis() - lastAppInfo) >= appInfoInterval) {
    options();
    Serial.print("$app,");
    Serial.print((active == true ? "1" : "0"));
    Serial.print(",");
    Serial.print((isi == true ? "1" : "0"));
    Serial.print(",");
    Serial.print((besleme == true ? "1" : "0"));
    Serial.print(",");
    Serial.print(procStart);
    Serial.print(",");
    Serial.print(fotoselTook);
    Serial.print(",");
    Serial.print(Status);
    Serial.print(",");
    Serial.print(correction);
    Serial.print(",");
    Serial.print(pkgcount);
    Serial.print(",");
    Serial.print(prodCount);
    Serial.print(",");
    Serial.print(prodTotal);
    Serial.print(",");
    Serial.print(ceneTook);
    Serial.print(",");
    Serial.print(cycleCount);
    Serial.print(",");
    Serial.print((feedMode == true ? "1" : "0"));
    Serial.print(",");
    Serial.print((drainMode == true ? "1" : "0"));
    Serial.print(",");
    Serial.print(dolumMax);
    Serial.print(",");
    Serial.print(tarihMax);
    Serial.print(",");
    Serial.print(paletMax);
    Serial.print(",");
    Serial.print(tarih_pull_proc);
    Serial.print(",");
    Serial.print(millis());
    Serial.println(";");
    lastAppInfo = millis();
  }
}

void feed(bool state) {
  if (state == true) {
    digitalWrite(inverter, LOW);
    digitalWrite(palet, LOW);
  } else {
    digitalWrite(palet, HIGH);
    digitalWrite(inverter, HIGH);
  }
}

void Stop() {

  digitalWrite(dolum, HIGH);
  digitalWrite(inverter, HIGH);
  digitalWrite(mikser, HIGH);
  digitalWrite(motor, HIGH);
  digitalWrite(palet, HIGH);
  digitalWrite(powerLed, HIGH);
  kesim = false;
  fsretry = 0;
  fotoselTookEx = 0;
  fotoselTook = 0;
  lastfotosel = 0;
  Fotosel = false;
  paletStart = 0;
  kesimFlip = 0;

  dolumStart = 0;
  procStart = 0;
  prodTotal += prodCount;
  Status = 0;
  active = false;
}

void Error(int code) {
  Serial.print("$err,");
  Serial.print(code);
  Serial.println(";");
  digitalWrite(buzzer, LOW);
  delay(300);
  digitalWrite(buzzer, HIGH);
  delay(300);

  digitalWrite(buzzer, LOW);
  delay(300);
  digitalWrite(buzzer, HIGH);
  delay(300);

  digitalWrite(buzzer, LOW);
  delay(300);
  digitalWrite(buzzer, HIGH);
  delay(300);
}

void Start() {
  //if (isi == true && ((millis() - isirun) >= isimin) || millis() < isimin) {
  //  Error(1);
  //  return;
  // }
  options();
  fsretry = 0;
  blipFor(25, 250);
  Status = 1;
  cycleCount = 0;
  pkgcount = 0;
  prodCount = 0;
  procStart = millis();
  fotoselTook = 0;
  fotoselTookEx = 0;
  lastfotosel = 0;
  Fotosel = false;
  kesimFlip = millis();

  digitalWrite(powerLed, LOW);
  digitalWrite(inverter, LOW);
  digitalWrite(motor, LOW);
  digitalWrite(palet, HIGH);
  digitalWrite(dolum, HIGH);
  active = true;
}

void options() {
  Serial.print("$opt,");
  Serial.print((fotosel_aktif == true ? "1" : "0"));
  Serial.print(",");
  Serial.print((dolum_aktif == true ? "1" : "0"));
  Serial.print(",");
  Serial.print((tarih_aktif == true ? "1" : "0"));
  Serial.println(";");
}

void checkSensors() {
  if (analogRead(isiSensor) >= analogMin) {
    if (isi == false) {
      isirun = millis();
    }
    isi = true;
  } else {
    isi = false;
  }

  if (analogRead(beslemeSensor) >= analogMin) {
    besleme = true;
  } else {
    besleme = false;
  }
  if (besleme != fbesleme) {
    beslemerun = millis();
  }
  fbesleme = besleme;
}

void toggle() {
  if (!active && QueueStop == false) {
    Start();
  } else {
    blipFor(100, 300);
    QueueStop = true;
    digitalWrite(mikser, HIGH);
    digitalWrite(powerLed, HIGH);
  }
}

boolean checkButton(int buttonPin) {
  int reading = digitalRead(buttonPin);
  unsigned long now = millis();

  if (reading != lastButtonState) {
    lastDebounceTime = now;
  }

  lastButtonState = reading;
  if ((now - lastDebounceTime) > debounceDelay && (now - lastTrigger >= toggleRate) && reading == LOW) {
    lastTrigger = now;
    return true;
  }
  return false;
}


void cutOff() {
  if (active == true) {
    return ;
  }
  digitalWrite(inverter, LOW);
  digitalWrite(motor, LOW);
  delay(2500);
  digitalWrite(inverter, HIGH);
  digitalWrite(motor, HIGH);
}
void blipFor(int rate, int duration) {
  blipstart = millis();
  blipduration = duration;
  bliprate = rate;
  blipactive = true;
}

void BLIP() {
  unsigned long now = millis();

  if ((now - blipstart) >= blipduration) {
    blipactive = false;
  }

  if (blipactive == false) {
    digitalWrite(buzzer, HIGH);
    return ;
  }

  if ((now - lastBlip) >= bliprate) {
    digitalWrite(buzzer, !digitalRead(buzzer));
    lastBlip = now;
  }
}

void paletBegin() {
  //  digitalWrite(inverter, LOW);
  digitalWrite(palet, LOW);
  paletStart = millis();
  Status = 2;
}

void paletEnd() {
  //  digitalWrite(inverter, HIGH);
  digitalWrite(palet, HIGH);
  _paletms = 0;
  tarihBegin();
}

void motorBegin() {
  Status = 1;
  digitalWrite(motor, LOW);
}

void tarihBegin() {
  digitalWrite(tarih_pin, LOW);
  digitalWrite(inverter, HIGH);
  tarihpinEnd = 0;
  tarihpinStart = millis();
  Status = 3;
}

void dolumBegin() {
  digitalWrite(dolum, LOW);
  dolumStart = millis();
  prodCount++;
  Status = 6;
}
