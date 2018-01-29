const int ledPin1 = 5;
const int ledPin2 = 10;
const int pwmWidth = 3;

byte data1[10] = {0x31, 0x82, 0x13, 0xE4, 0x75,
                  0x66, 0x27, 0xA8, 0x49, 0xDA};
unsigned dataNumber = 0;

int counter1 = 0;
const int packet1Size = 31;
bool led1Status = false;
byte packet1[packet1Size];

void initializeTimer()
{
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  int freq = 3000;
  int value = 16000000 / 256 /  freq / 2 ; // timer is 8 bits -> 256 values

  OCR1A = value;            // compare match register 16MHz/256/2Hz
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
      digitalWrite(ledPin1,led1Status);
    }
    
    if(packet1[counter1] == 0 && led1Status == true)
    {
      led1Status = false;
      digitalWrite(ledPin1,led1Status);
    }
 }

  counter1++;
  if(counter1 == packet1Size)
  {
     counter1 = 0;
     dataNumber++;
     if(dataNumber > 9)
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
  while(i<8)
  {
    if(j<4)
    {
      packet1[j++] = 0;
      continue;
    }
    
    byte tmp = (data1[dataNumber]) & (0x80>>i);
    if(tmp == 0)
    {
      packet1[j++] = 1; //part 1 of data
      packet1[j++] = 0; //part 2 of data
      packet1[j++] = 0; //part 3 of data
      i++;
    }
    else
    {
      packet1[j++] = 1; //part 1 of data
      packet1[j++] = 1; //part 2 of data
      packet1[j++] = 0; //part 3 of data
      i++;
    }

    if(i==8)
    {
      if(parity(data1[dataNumber]))
      {
        packet1[j++] = 1; //part 1 of parity
        packet1[j++] = 1; //part 2 of parity
        packet1[j++] = 0; //part 3 of parity
      }
      else
      {
        packet1[j++] = 1; //part 1 of parity
        packet1[j++] = 0; //part 2 of parity
        packet1[j++] = 0; //part 3 of parity
      }
    }
  }
//  printPacket();
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
//  printPacket();
  initializeTimer();
}

void loop(){}

