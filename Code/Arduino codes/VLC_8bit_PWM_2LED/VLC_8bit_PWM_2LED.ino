const int ledPin1 = 11;
const int ledPin2 = 10;
const int dataLength = 10;
const int pwmWidth = 3;
const int silentZone = 2 * pwmWidth;

int counter1 = 0;
int dataArray1[dataLength];
int *packet1;
int packet1Size;
bool led1Status = false;

int counter2 = 0;
int dataArray2[dataLength];
int *packet2;
int packet2Size;
bool led2Status = false;

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
  //For LED1
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
  else if(counter1 < packet1Size+silentZone)
  {
    led1Status = false;
    digitalWrite(ledPin1,led1Status);
  }

  counter1++;
  if(counter1 == packet1Size+silentZone)
    counter1 = 0;

  //For LED2
  if(counter2 < packet2Size)
  {
    if(packet2[counter2] == 1 && led2Status == false)
    {
      led2Status = true;
      digitalWrite(ledPin2,led2Status);
    }
    
    if(packet2[counter2] == 0 && led2Status == true)
    {
      led2Status = false;
      digitalWrite(ledPin2,led2Status);
    }
  }
  else if(counter2 < packet2Size+silentZone)
  {
    led2Status = false;
    digitalWrite(ledPin2,led2Status);
  }
  
  counter2++;
  if(counter2 == packet2Size+silentZone)
    counter2 = 0;
}

void setData1()
{
  //StartBIT
  dataArray1[0] = 1;
  //Data
  dataArray1[1] = 0;
  dataArray1[2] = 1;
  dataArray1[3] = 0;
  dataArray1[4] = 1;
  dataArray1[5] = 0;
  dataArray1[6] = 1;
  dataArray1[7] = 0;
  dataArray1[8] = 1;
  //Parity -- even
  dataArray1[9] = 0;
}

void makePacket1()
{
  packet1Size = dataLength * pwmWidth;
  packet1 = (int*) malloc(packet1Size * sizeof(int));
  int i=0,j=0;
  while(i<dataLength)
  {
    if(dataArray1[i] == 1)
    {
      packet1[j++] = 1; //part 1 of data
      packet1[j++] = 1; //part 2 of data
      packet1[j++] = 0; //part 3 of data
      i++;
    }
    else
    {
      packet1[j++] = 1; //part 1 of data
      packet1[j++] = 0; //part 2 of data
      packet1[j++] = 0; //part 3 of data
      i++;
    }
  }
}

void setData2()
{
  //StartBIT
  dataArray2[0] = 1;
  //Data
  dataArray2[1] = 1;
  dataArray2[2] = 1;
  dataArray2[3] = 1;
  dataArray2[4] = 1;
  dataArray2[5] = 1;
  dataArray2[6] = 1;
  dataArray2[7] = 1;
  dataArray2[8] = 1;
  //Parity -- even
  dataArray2[9] = 0;
}

void makePacket2()
{
  packet2Size = dataLength * pwmWidth;
  packet2 = (int*) malloc(packet2Size * sizeof(int));
  int i=0,j=0;
  while(i<dataLength)
  {
    if(dataArray2[i] == 1)
    {
      packet2[j++] = 1; //part 1 of data
      packet2[j++] = 1; //part 2 of data
      packet2[j++] = 0; //part 3 of data
      i++;
    }
    else
    {
      packet2[j++] = 1; //part 1 of data
      packet2[j++] = 0; //part 2 of data
      packet2[j++] = 0; //part 3 of data
      i++;
    }
  }
}

void setup()
{
    pinMode(ledPin1, OUTPUT);
    pinMode(ledPin2, OUTPUT);
    setData1();
    setData2();
    makePacket1();
    makePacket2();
    delay(500);
//    digitalWrite(ledPin2,LOW);
    initializeTimer();
}

void loop(){}


