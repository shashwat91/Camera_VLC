// On-Off frequency should be in ratio 3:2 (or 40%-60% duty cycle)
const int freq_on = 1000;
const int freq_off = 666;
int value_on = 16000000 / 256 /  freq_on / 2;
int value_off = 16000000 / 256 /  freq_off / 2;
const int value[2] = {value_off,value_on};

const int ledPin1 = 5;
const int ledPin2 = 10;
const int pwmWidth = 3;

byte data1[10] = {0x00, 0x1, 0x2, 0x3, 0x4, 0x5,
                  0x6, 0x7, 0x8, 0x9};
unsigned dataNumber = 0;

int counter1 = 0;
const int packet1Size = 21;
bool led1Status = false;
byte packet1[packet1Size];

void initializeTimer()
{
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  OCR1A = value[led1Status];            // compare match register 16MHz/256/2Hz
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
     dataNumber++;
     if(dataNumber > 7)
      dataNumber=0;
     makePacket1();
  }
}

byte parity(byte x)
{
   byte y;
   y = x ^ (x >> 1);
   y = y ^ (y >> 2);
   y = y ^ (y >> 4);
   y = y ^ (y >> 8);
   y = y ^ (y >>16);
   return y & 1;
}

void makePacket1()
{
  int i=0,j=0;
  while(i<3)
  {
    if(i==0)
    {
      packet1[j++] = 1;
      packet1[j++] = 0;
      packet1[j++] = 0;
      packet1[j++] = 0;
      packet1[j++] = 1;

    }
    
    byte tmp = (data1[dataNumber]) & (0x04>>i);
    if(tmp == 0)
    {
      packet1[j++] = 1; //part 1 of data
      packet1[j++] = 0; //part 2 of data
      packet1[j++] = 0; //part 3 of data
      packet1[j++] = 1; //part 3 of data
      i++;
    }
    else
    {
      packet1[j++] = 1; //part 1 of data
      packet1[j++] = 1; //part 2 of data
      packet1[j++] = 0; //part 3 of data
      packet1[j++] = 0; //part 3 of data
      i++;
    }

    if(i==3)
    {
        packet1[j++] = 1; //part 1 of parity
        packet1[j++] = 0; //part 2 of parity
        packet1[j++] = 0; //part 3 of parity
        packet1[j++] = 1; //part 3 of data
    }
  }
  //printPacket();
}

void printPacket()
{
  Serial.print(data1[dataNumber],HEX);
  Serial.print("::");
  int i=0;
  while(i<packet1Size)
  {
    Serial.print(packet1[i]);
    i++;
  }
  Serial.println();
}

void setup()
{
  makePacket1();
  pinMode(ledPin1, OUTPUT);
  Serial.begin(115200);
  delay(5000);
  printPacket();
  initializeTimer();
}

void loop(){}

