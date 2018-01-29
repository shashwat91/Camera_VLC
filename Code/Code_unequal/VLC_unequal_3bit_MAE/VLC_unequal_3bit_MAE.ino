const int freq_on = 3000;
const int freq_off = 2000;
int value_on = 16000000 / 256 /  freq_on / 2;
int value_off = 16000000 / 256 /  freq_off / 2;
const int value[2] = {value_off,value_on};

const int ledPin1 = 5;
const int ledPin2 = 10;
const int pwmWidth = 2;

byte data1[10] = {0x1, 0x2, 0x3, 0x4, 0x5,
                  0x6, 0x7, 0x8, 0x9, 0xA};
unsigned dataNumber = 7;

int counter1 = 4;
const int packet1Size = 10;
bool led1Status = false;
byte packet1[packet1Size];

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
    // -- Switch led on if its off and current bit is 1
    if(packet1[counter1] == 1 && led1Status == false)
    {
      led1Status = true;
      OCR1A = value[led1Status];
      digitalWrite(ledPin1,led1Status);
    }
    // -- Switch led off if its on and current bit is 0
    else if(packet1[counter1] == 0 && led1Status == true)
    {
      led1Status = false;
      OCR1A = value[led1Status];
      digitalWrite(ledPin1,led1Status);
    }
  }
  counter1++; // Go to next bit
  
  //reset counter and make next packet
  if(counter1 == packet1Size)
  {
     counter1 = 0;
     dataNumber++;
     if(dataNumber > 9)
      dataNumber=0;
     makePacket1();
  }
}

// Function for parity check
byte parity(byte x)
{
   byte y;
   y = x ^ (x >> 1);
   y = y ^ (y >> 2);
   y = y ^ (y >> 4);
   y = y ^ (y >> 8);
   //y = y ^ (y >>16);
   return y & 1;
}

void makePacket1()
{
  int i=0,j=0;
  while(i<3)
  {
    if(j<3)   // --first 3 bits of start of packet
    {
      packet1[j++] = 0;
      continue;
    }

    if(j==3)    // 4th bit start
    {
      packet1[j++] = 1;
      continue;
    }
    
    byte tmp = (data1[dataNumber]) & (0x04>>i);
    if(tmp == 0)  // -- bits for symbol 0
    {
      packet1[j++] = 1; //part 1 of data
      packet1[j++] = 0; //part 2 of data
      i++;
    }
    else    // -- bits for symbol 1
    {
      packet1[j++] = 0; //part 1 of data
      packet1[j++] = 1; //part 2 of data
      i++;
    }
  }
//  printPacket();
}

// Function for printing packets through serial port
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

