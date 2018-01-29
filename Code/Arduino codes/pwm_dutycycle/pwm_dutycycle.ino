bool sta = false;
bool sta2= false;

void initTimer1()
{
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  int freq = 1500; // frequency for PWM wave on OCR1B
  unsigned long OCR1A_val = 16000000 / 8 / freq / 2;

  int dCycle = 30; // duty cycle for PWM wave on OCR1B
  unsigned long OCR1B_val = (OCR1A_val*dCycle) / 100;

  // _BV(COM1A1) -> non-inverting mode, clear  OC1A/OC1B on Compare Match.
  // WGM13 + WGM11 + WGM10 -> PWM phase correct, OCR1A as TOP 
  TCCR1A = _BV(COM1A0) | _BV(COM1B1) | _BV(WGM11) | _BV(WGM10);
  TCCR1B = _BV(WGM13) | _BV(CS11); // 8 prescaler
  OCR1A = OCR1A_val;     // Timer1A compare value -- for PWM frequency
  OCR1B = OCR1B_val;     // Timer1B compare value -- for duty cycle
  TIMSK1 |= _BV(OCIE1A) | _BV(OCIE1B); // enable timer1A compare interrupt
  interrupts();
}

ISR(TIMER1_COMPA_vect)
{
  Serial.println("timer1A");
  digitalWrite(13,sta);
  sta = !sta;
}
ISR(TIMER1_COMPB_vect)
{
  Serial.println("timer1B");
  digitalWrite(6,sta2);
  sta2 = !sta2;
}
ISR(TIMER1_CAPT_vect)
{
  digitalWrite(6,sta2);
  sta2 = !sta2;
}

void setup()
{
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  pinMode(10,OUTPUT);   // OCR1B output
  pinMode(9,OUTPUT);    // OCR1A output
  pinMode(6,OUTPUT);
  delay(1000);
  initTimer1();
}

void loop() {}
