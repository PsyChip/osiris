#ifndef config_h
#define config_h

#include "Arduino.h"

#define rot_inc 3                      // rotary encoder increase
#define rot_dec 2                      // rotary encoder decrease

#define out_inverter 4                 // relay out for inverter
#define out_motor 5                    // main engine relay
#define out_palet 6                    // palet engine relay
#define out_dolum 7                    // dolum engine relay
#define out_mikser 8                   // mixer relay
#define out_besleme 9                  // besleme rölesi

#define out_tarih_isi 10               // tarih isitici
#define out_tarih_push 11              // tarih baski rölesi
#define out_tarih_pull 12              // tarih ribbon cekme
#define out_buzzer 13                  // tarih ribbon cekme

#define in_fotosel A0                  // fotosel sensörü
#define in_cene_kapali A1              // çene pozisyon sensörü
#define in_cene_acik A2                // çene kapalı
#define in_besleme A3                  // besleme dolum sensörü

#define btn_start A4                   // aç kapa butonu
//#define btn_feed A5                    // palet kaydırma butonu

#define conf_boy 1
#define conf_gramaj 2
#define conf_tarih 3
#define conf_ribbon 4

#define opt_fotosel 1
#define opt_dolum 2
#define opt_tarih 3
#define opt_besleme 4

#define DEBOUNCE_DELAY_BUTTON 50                 // obvious
#define STOP_TIMEOUT 5000               
#define TARIH_DELAY 50                           // time between push and pull

const unsigned int min_value[4] = {
  100, 50, 50, 50
};

const unsigned int max_value[4] = {
  3000, 5000, 2000, 2000
};


#endif
