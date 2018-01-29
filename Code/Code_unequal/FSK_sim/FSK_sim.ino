const int ledPin1 = 5;

const int freq_on = 5000;
const int freq_off = 3334;
int value_on = 16000000 / 256 /  freq_on / 2;
int value_off = 16000000 / 256 /  freq_off / 2;
const int value[2] = {value_off,value_on};

int counter1 = 0;
const int packet1Size = 58;
bool led1Status = false;
byte packet1[packet1Size] = {1,0,1,0,1,0, 0,0, 1,1, 1,1, 0,0, 0,0, 1,1, 1,1, 0,0, 1,0,1,0,1,0, 1,1, 0,0, 0,0, 1,1, 0,0, 1,1, 0,0, 1,1, 1,0,1,0,1,0, 1,1, 0,0, 0,0, 1,1};

void initializeTimer()
{
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  OCR1A = value[led1Status];      // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_COMPA_vect)
{
 if(counter1 < packet1Size)
 { 
    if(packet1[counter1] == 1 && led1Status == false)
    {
      led1Status = true;
      OCR1A = value[led1Status];
      digitalWrite(ledPin1,led1Status);
    }
    
    if(packet1[counter1] == 0 && led1Status == true)
    {
      led1Status = false;
      OCR1A = value[led1Status];
      digitalWrite(ledPin1,led1Status);
    }
 }

  counter1++;
  if(counter1 == packet1Size)
  {
     counter1 = 0;
  }
}


void setup() {
pinMode(ledPin1, OUTPUT);
initializeTimer();
}

void loop() {}
