

#include <rpcWiFi.h>
#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
GAS_GMXXX<TwoWire> gas;
#include "SAMD_ISR_Servo.h"

#define ISR_SERVO_DEBUG             1
#define USING_SAMD_TIMER            TIMER_TC3
#define MIN_MICROS        500
#define MAX_MICROS        2500 

#define SERVO_PIN_1       A5
#define SERVO_PIN_2       A3


typedef struct
{
  int     servoIndex;
  uint8_t servoPin;
} ISR_servo_t;

#define NUM_SERVOS            2
ISR_servo_t ISR_servo[] =
{
  { -1, SERVO_PIN_1 }, { -1, SERVO_PIN_2 }
};



const char *ssid = "Tufts_Wireless";
//Servo lServo;
//Servo rServo;
String header;
int pos = 0;


float no2 = 0;
float c2h5ch = 0;
float voc = 0;
float co = 0;

WiFiServer server(80);

void setup()
{
  gas.begin(Wire, 0x08);
  

    pinMode(ISR_servo[0].servoPin, OUTPUT);
    pinMode(ISR_servo[1].servoPin, OUTPUT);
//    digitalWrite(ISR_servo[0].servoPin, LOW);
//    digitalWrite(ISR_servo[1].servoPin, LOW);

  Serial.begin(115200); 
  
  while (!Serial);
  Serial.print(F("\nStarting SAMD_MultipleRandomServos on ")); Serial.println(BOARD_NAME);
  Serial.println(SAMD_ISR_SERVO_VERSION);
  SAMD_ISR_Servos.useTimer(USING_SAMD_TIMER);
  
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    ISR_servo[index].servoIndex = SAMD_ISR_Servos.setupServo(ISR_servo[index].servoPin, MIN_MICROS, MAX_MICROS);

    if (ISR_servo[index].servoIndex != -1)
    {
      Serial.print(F("Setup OK Servo index = ")); Serial.println(ISR_servo[index].servoIndex);
    }
    else
    {
      Serial.print(F("Setup Failed Servo index = ")); Serial.println(ISR_servo[index].servoIndex);
    }
  }

  SAMD_ISR_Servos.setReadyToRun();

  delay(200);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());

  server.begin();
}

int value = 0;


void printServoInfo(int indexServo)
{
  Serial.print(F("Servos idx = "));
  Serial.print(indexServo);
  Serial.print(F(", act. pos. (deg) = "));
  Serial.print(SAMD_ISR_Servos.getPosition(ISR_servo[indexServo].servoIndex) );
  Serial.print(F(", pulseWidth (us) = "));
  Serial.println(SAMD_ISR_Servos.getPulseWidth(ISR_servo[indexServo].servoIndex));
}

void loop()
{

  no2 = gas.getGM102B();
  c2h5ch = gas.getGM302B();
  voc = gas.getGM502B();
  co = gas.getGM702B();
  Serial.end();
  
  WiFiClient client = server.available(); // listen for incoming clients

   if (client) {
    //Serial.println("New client");
    String currLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        header += c;
        
        if (c == '\n') {
          if(currLine.length() == 0){
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connection: close");  
            client.println();

            client.println("<!DOCTYPE HTML>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<html>");
            client.println("<body style = \"background-color:lightslategrey;\">");
            client.println(no2);
            client.println("<table>");
            client.println("<tr><td colspan=\"3\" align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('forward');\" ontouchstart=\"toggleCheckbox('forward');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Forward</button></td></tr>");
            client.println("<tr><td align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('left');\" ontouchstart=\"toggleCheckbox('left');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Left</button></td><td align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('stop');\" ontouchstart=\"toggleCheckbox('stop');\">Stop</button></td><td align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('right');\" ontouchstart=\"toggleCheckbox('right');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Right</button></td></tr>");
            client.println("<tr><td colspan=\"3\" align=\"center\"><button class=\"button\" onmousedown=\"toggleCheckbox('backward');\" ontouchstart=\"toggleCheckbox('backward');\" onmouseup=\"toggleCheckbox('stop');\" ontouchend=\"toggleCheckbox('stop');\">Backward</button></td></tr>");
            client.println("</table>");                   
            client.println("<script>");
            client.println("function toggleCheckbox(x) {");
            client.println("var xhr = new XMLHttpRequest();");
            client.println("xhr.open(\"GET\", \"/\" + x, true);");
            client.println("xhr.send();");
            client.println("}");
            client.println("</script>");
            client.println("</body>");
            client.println("</html>");  

            if (header.indexOf("forward") != -1){
              SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 0 );
              SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 180 );
             
            }
             
            
            if (header.indexOf("stop") != -1){
              SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 90 );
              SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 90 );
              
            }
            if (header.indexOf("left") != -1){
              SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 0 );
              SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 0 );

  
            }

            if (header.indexOf("right") != -1){
              SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 180 );
              SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 180 );
            }  

            if (header.indexOf("back") != -1){
              SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 180 );
              SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 0 );

            }
                 
            client.println();
            break;
          }
          else{
            currLine = "";
          }
        }
        else if (c != '\r'){
          currLine += c;
        }
      }
    }
    header = "";
    client.stop();
    //Serial.println("Client Disconnected.");
  }
}