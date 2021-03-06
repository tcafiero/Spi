#include <Wire.h>
#include <stdio.h>

#define CURRENT_SENSOR_ADAPTOR0  1
#define INITIAL_SAMPLES         100
#define SAMPLES        32
#define CONVERSION_TIME 1              // the right value would be 1/10000*1000

byte CurrentSensor[] = {A0, A1, A2, A3, A6, A7};
int Current[sizeof(CurrentSensor)];
long CurrentSum[sizeof(CurrentSensor)];
int AverageCurrent[sizeof(CurrentSensor)];
byte Signal[sizeof(CurrentSensor)];
byte Edge[sizeof(CurrentSensor)];
byte sample;



int myputc(char c, FILE *)
{
  Serial.write(c);
  return 0;
}


void setup() {
  byte i;
  Serial.begin(115200);
  fdevopen(&myputc, NULL);
  //  for (i = 0; i < sizeof(CurrentSensor) ; i++)
  //    pinMode(CurrentSensor[i], INPUT);
  delay(1000);
  for (i = 0; i < sizeof(CurrentSensor) ; i++)
    CurrentSum[i] = 0;
  for (i = 0; i < INITIAL_SAMPLES; i++)
  {
    for (byte j = 0; j < sizeof(CurrentSensor) ; j++)
      CurrentSum[j] += analogRead(CurrentSensor[j]);
    delay(CONVERSION_TIME);
  }
  for (i = 0; i < sizeof(CurrentSensor) ; i++)
  {
    AverageCurrent[i] = CurrentSum[i] / INITIAL_SAMPLES;
    CurrentSum[i] = 0;
    Current[i] = 0;
    Signal[i] = 0;
    Edge[i] = 0;
  }
  sample = 0;
  Wire.begin(CURRENT_SENSOR_ADAPTOR0);                // join i2c bus with address
  Wire.onReceive(receiveEvent); // register event
}

void loop() {
  if (sample >= SAMPLES)
  {
    for (byte j = 0; j < sizeof(CurrentSensor) ; j++)
    {
      Current[j] = CurrentSum[j] / sample;
      CurrentSum[j] = 0;
      int delta = abs(Current[j] - AverageCurrent[j]);
      if (delta >= 5)
        if (Signal[j] == 0)
        {
          Signal[j] = 1;
          Edge[j] = 1;
        }
      if (delta < 3)
        if (Signal[j] == 1)
        {
          Signal[j] = 0;
          Edge[j] = -1;
        }
        else Edge[j] = 0;
    }
    sample = 0;
  }
  else
  {
    for (byte j = 0; j < sizeof(CurrentSensor) ; j++)
    {
      unsigned int value;
      value = analogRead(CurrentSensor[j]);
      CurrentSum[j] += value;
    }
    sample++;
    delay(CONVERSION_TIME);
  }
}

void SendWave()
{
  Wire.write((char*)Signal, sizeof(Signal));
}

void SendEdge()
{
  Wire.write((char*)Signal, sizeof(Edge));
}


void SendRaw()
{
  Wire.write((char*)Current, sizeof(Current));
}

void receiveEvent(int num)
{
  char command;
  while (Wire.available())
  {
    command = Wire.read();
    switch (command)
    {
      case 'c':
        break;
      case 'd':
        Wire.onRequest(SendRaw);
        break;
      case 'w':
        Wire.onRequest(SendWave);
        break;
      case 'e':
        Wire.onRequest(SendEdge);
        break;
      default:
        break;
    }
  }
}

