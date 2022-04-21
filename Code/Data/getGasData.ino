
#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
#include <Arduino_KNN.h>
GAS_GMXXX<TwoWire> gas;

int N02;
int C2H5CH;
int VOC;
int CO;

int freq = 1000;

String dataLabel1 = "N02";
String dataLabel2 = "C2H5CH";
String dataLabel3 = "VOC";
String dataLabel4 = "CO";
bool label = true;

void setup() {
  gas.begin(Wire, 0x08); // use the hardware I2C
  
  Serial.begin(9600);
  delay(5000);
  
}

void loop() {
  
  while(label){ //runs once
          Serial.print(dataLabel1);
          Serial.print(",");
          Serial.print(dataLabel2);
          Serial.print(",");
          Serial.print(dataLabel3);
          Serial.print(",");
          Serial.println(dataLabel4);
          label=false;
   }

  N02 = gas.getGM102B();
  C2H5CH = gas.getGM302B();
  VOC = gas.getGM502B();
  CO = gas.getGM702B();

  Serial.print(N02);
  Serial.print(",");
  Serial.print(C2H5CH);
  Serial.print(",");
  Serial.print(VOC);
  Serial.print(",");
  Serial.println(CO);

  delay(freq);

 
  }
 
