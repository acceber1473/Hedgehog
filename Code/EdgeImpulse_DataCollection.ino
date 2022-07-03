#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
GAS_GMXXX<TwoWire> gas;


void setup() {
  // put your setup code here, to run once:
  gas.begin(Wire, 0x08); // use the hardware I2C
  Serial.begin(115200);
}

void loop() {
  
  String NO2 = String(gas.getGM102B());
  String C2H = String(gas.getGM302B());
  String VOC = String(gas.getGM502B());
  String CO = String(gas.getGM702B());
  Serial.println(NO2+","+C2H+","+VOC+","+CO);

  delay(10);

  
}
