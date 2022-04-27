#include <OneWire.h>
#include <Wire.h>
#define SensorPin 0          //pH meter Analog output to Arduino Analog Input 0


unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;
int DS18S20_Pin = 4;
OneWire ds(DS18S20_Pin);
String json;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  
  Serial.println("Ready");    //Test the serial monitor
  // Send data through I2C
  Wire.begin(0x55);
  Wire.onRequest(requestEvent);
}

void requestEvent() {
  Wire.write(json.c_str());
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value

  //Serial.print(phValue);
  json = "{";
  json += "\"ph\":" + String(phValue, 2) + ",";
  json += "\"temp\":" + String(getTemp(),2);
  json += "}";  
  
  Serial.println(json);  
  //Wire.write(json.c_str());
    
  digitalWrite(13, HIGH);       
  delay(1000);
  digitalWrite(13, LOW); 
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;

}
