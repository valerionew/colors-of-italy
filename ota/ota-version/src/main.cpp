#include <Arduino.h>
#define VERSION "1.0.0"

void setup()
{
  Serial.begin(115200);
}

void loop()
{
  Serial.print("UPDATED! Version: ");
  Serial.print(VERSION);
  Serial.println();
  delay(1000);
}