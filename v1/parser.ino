#define CMLEN 16

String buffer;
String command;
String param;
int fstep;

const String clist[CMLEN] = {"OK+CONN", "OK+LOST", "toggle", "feed", "drain", "tarihtest", "topt", "skgram", "skboy", "sktarih", "cenetest", "sktpull", "sktwait", "cfotosel", "cdolum", "ctarih"};

void flush() {
  command = "";
  param = "";
  buffer = "";
}

unsigned long lastcmd = 0;
const unsigned long timeout = 2500;

bool ParserCheckIn() {
  if ((millis() - lastcmd) > timeout && lastcmd > 0) {
    Serial.flush();
    flush();
  }

  if (Serial.available() > 0) {
    lastcmd = millis();
    if (parseCommand() == true) {
      handleCommand();
      flush();
      lastcmd = 0;
    }
    return true;
  }
  return false;
}

boolean parseCommand() {
  char incomingByte = (char)Serial.read();
  if (incomingByte == 10 || incomingByte == 13) {
    Serial.flush();
    fstep = 0;
    int bl = buffer.length();
    if (bl < 4) {
      flush();
      return false;
    }

    for (int i = 0; i < bl; i++) {
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
  } else {
    buffer += incomingByte;
  }
  return false;
}

int findCID() {
  for (int i = 0; i < CMLEN; i++) {
    if (command.startsWith(clist[i])) {
      return i;
    }
  }
  return -1;
}


void handleCommand() {
  //  connected = true;

  switch (findCID()) {
    case 0: // bluetooth connected
      app_connected = true;
      options();
      break;
    case 1:
      app_connected = false;
      break;
    case 2:
      toggle();
      appInfo(true);
      break;
    case 3:
      if (active == false) {
        feedMode = !feedMode;
        if (feedMode == true) {
          digitalWrite(inverter, LOW);
          digitalWrite(palet, LOW);
        } else {
          digitalWrite(inverter, HIGH);
          digitalWrite(palet, HIGH);
        }
      }
      break;
    case 4:
      if (active == false) {
        drainMode = !drainMode;
        if (drainMode == true) {
          digitalWrite(mikser, LOW);
          digitalWrite(dolum, LOW);
        } else {
          digitalWrite(mikser, HIGH);
          digitalWrite(dolum, HIGH);
        }
      }
      break;
    case 5:
      if (active == false) {
        digitalWrite(tarih_pin, LOW);
        delay(tarihMax);
        digitalWrite(tarih_pin, HIGH);
        delay(tarih_pull_delay);
        digitalWrite(tarih_pull, LOW);
        delay(tarih_pull_proc);
        digitalWrite(tarih_pull, HIGH);
      }
      break;
    case 6:
      switch (param.toInt()) {
        case 1:
          fotosel_aktif = !fotosel_aktif;
          break;
        case 2:
          dolum_aktif = !dolum_aktif;
          break;
        case 3:
          tarih_aktif = !tarih_aktif;
          break;
      }
      options();
      break;
    case 7:
      dolumMax = param.toInt();
      if (dolumMax < dolum_min) {
        dolumMax = dolum_min;
      }
      if (dolumMax > dolum_max) {
        dolumMax = dolum_max;
      }
      break;
    case 8:
      paletMax = param.toInt();

      if (paletMax < palet_min) {
        paletMax = palet_min;
      }
      if (paletMax > palet_max) {
        paletMax = palet_max;
      }

      break;
    case 9:
      tarihMax =  param.toInt();

      if (tarihMax < tarih_min) {
        tarihMax = tarih_min;
      }
      if (tarihMax > tarih_max) {
        tarihMax = tarih_max;
      }
      break;
    case 10:
      cutOff();
      delay(500);
      break;
    case 11:
      tarih_pull_proc =  param.toInt();
      if (tarih_pull_proc < tarih_min) {
        tarih_pull_proc = tarih_min;
      }
      if (tarih_pull_proc > tarih_max) {
        tarih_pull_proc = tarih_max;
      }
      break;
    case 12:
      tarih_pull_delay =  param.toInt();

      if (tarih_pull_delay < tarih_min) {
        tarih_pull_delay = tarih_min;
      }
      if (tarih_pull_delay > tarih_max) {
        tarih_pull_delay = tarih_max;
      }
      break;
    case 13:
      if (param.toInt() == 1) {
        fotosel_aktif = true;
      } else {
        fotosel_aktif = false;
      }

      break;
      case 14:
       if (param.toInt() == 1) {
        dolum_aktif = true;
      } else {
        dolum_aktif = false;
      }

      break;
      case 15:
        if (param.toInt() == 1) {
        tarih_aktif = true;
      } else {
        tarih_aktif = false;
      }
      break;
        default:
          Serial.print("$unk,");
          buffer.trim();
          Serial.print(buffer);
          Serial.println(";");
      }
  }
