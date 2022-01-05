
// Watchdog Initialization
void InitLowPower() {
  ADCSRA = 0;
  power_adc_disable ();
  power_spi_disable();
}

void WatchdogInit() {
  MCUSR = MCUSR & B11110111;
  WDTCSR = WDTCSR | B00011000;
  WDTCSR = B00100001;
  WDTCSR = WDTCSR | B01000000;
  MCUSR = MCUSR & B11110111;
//  wdt_reset();
}

// Watchdog Triggered
ISR(WDT_vect) {
  p->Terminate();
}
