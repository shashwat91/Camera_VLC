/** FSK packet
 *  | SS  | D1  | SS  | D2  | SS  | P   |  
 *    Fs    F1    Fs    F2    Fs    F3
 *    
 *    Frequency states:
 *    Fs -> 0
 *    F1 -> 1
 *    F2 -> 2
 *    F3 -> 3
 *    
 *  Each Data is 4 symbols (2 bit encoded by Manchester encoding)
 */

const int freqS_on  = 5000; // Fs = 5000 Hz
const int freqS_off = 3333;
const int freq1_on  = 2500; // F1 = 2500 Hz 
const int freq1_off = 1666;
const int freq2_on  = 2000; // F2 = 2000 Hz
const int freq2_off = 1333;
const int freq3_on  = 1500; // F3 = 1500 Hz
const int freq3_off = 1000;

const int value[4][2] = {16000000 / 256 /  freqS_off, 16000000 / 256 /  freqS_on,
                         16000000 / 256 /  freq1_off, 16000000 / 256 /  freq1_on,
                         16000000 / 256 /  freq2_off, 16000000 / 256 /  freq2_on,
                         16000000 / 256 /  freq3_off, 16000000 / 256 /  freq3_on};
enum frequencies {Fs, F1, F2, F3};
frequencies freq_status = Fs;
int data_ptr = 0;

const int ledPin = 5;
const int symbols_per_bit = 2;
const int Fs_sym = 6;
const int data_sym = 4;
const int preamble[Fs_sym] = {1,0,1,0,1,0};

byte data = 0x06;
unsigned dataNumber = 0;
int counter = 0;
const int packetSize = 30;
bool ledStatus = false;
byte packet[packetSize];

void initializeTimer()
{
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  OCR1A = value[freq_status][ledStatus];      // compare match register 16MHz/256/2Hz
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS12);    // 256 prescaler
  TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
  interrupts();             // enable all interrupts
}

ISR(TIMER1_COMPA_vect)
{
  if(counter==30)
    return;
  if(counter < 6)
    freq_status = Fs;
  else if(counter < 10)
    freq_status = F1;
  else if(counter < 16)
    freq_status = Fs;
  else if(counter < 20)
    freq_status = F2;
  else if(counter < 26)
    freq_status = Fs;
  else
    freq_status = F3;
    
  //Serial.println(freq_status);

 if(counter < packetSize)
  { 
    // -- Switch led on if its off and current bit is 1
    if(packet[counter] == 1)
    {
      ledStatus = true;
      OCR1A = value[freq_status][ledStatus];
      digitalWrite(ledPin,ledStatus);
    }
    // -- Switch led off if its on and current bit is 0
    else if(packet[counter] == 0)
    {
      ledStatus = false;
      OCR1A = value[freq_status][ledStatus];
      digitalWrite(ledPin,ledStatus);
    }
  }
  counter++; // Go to next bit
  
  //reset counter and make next packet
  if(counter == packetSize)
  {
     //counter = 0;
     //makePacket();
  }
}

byte parity(byte x)
{
   byte y;
   y = x ^ (x >> 1);
   y = y ^ (y >> 2);
   y = y ^ (y >> 4);
   return y & 1;
}

void makePacket()
{
  int i=0,j;
  data_ptr = 0;
  byte tmp_data = data;
  while(i<packetSize)
  {
    j=0;
    if(i == 0 || i == 10 || i == 20)
      while(j < Fs_sym)
      {
        packet[i++] = preamble[j++];
      }

    if(i==26)
    {
      tmp_data = parity(data);
      data_ptr = 2;
    }
     
    byte tmp = (tmp_data) & (0x08>>(data_ptr));
    
    if(tmp == 0)  // -- bits for symbol 0
    {
      packet[i++] = 1; //part 1 of data
      packet[i++] = 0; //part 2 of data
      data_ptr++;
    }
    else    // -- bits for symbol 1
    {
      packet[i++] = 0; //part 1 of data
      packet[i++] = 1; //part 2 of data
      data_ptr++;
    }
  }
}

void printPacket()
{
  for(int i=0;i<packetSize;++i)
  {
    if(i==6 || i==10 || i==16 || i==20 || i==26)
      Serial.print("  ");
    Serial.print(packet[i]);
  }
  Serial.println();
}

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin,OUTPUT);
  makePacket();
  printPacket();
  delay(5000);
  initializeTimer();
}

void loop()
{}
