#include <rpcWiFi.h>
#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
#include "SAMD_ISR_Servo.h"
#include <Arduino_KNN.h>
#include "TFT_eSPI.h"



#define ISR_SERVO_DEBUG             1
#define USING_SAMD_TIMER            TIMER_TC3
#define MIN_MICROS        500
#define MAX_MICROS        2500 

#define SERVO_PIN_1       A5
#define SERVO_PIN_2       A3

TFT_eSPI tft;
GAS_GMXXX<TwoWire> gas;


#define num_Inputs 4
#define num_Classes 5
#define num_Neighbors 2
float sensorData[num_Inputs];
int classification;
String message = "Please Start training";


String webPage, header;

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



const char *ssid = "yourNetwork";
const char *pass = "secretPassword"; 
//Servo lServo;
//Servo rServo;
int pos = 0;


float no2 = 0;
float c2h5ch = 0;
float voc = 0;
float co = 0;

WiFiServer server(80);
KNNClassifier myKNN(num_Inputs);


void setup()
{
  gas.begin(Wire, 0x08);
  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_WHITE);
  tft.setTextColor(TFT_BLACK);          
  tft.setTextSize(2);
  tft.drawString("Initializing...", 30, 100);
  tft.drawString("Please Wait", 30, 120);
  

  pinMode(ISR_servo[0].servoPin, OUTPUT);
  pinMode(ISR_servo[1].servoPin, OUTPUT);
  digitalWrite(ISR_servo[0].servoPin, LOW);
  digitalWrite(ISR_servo[1].servoPin, LOW);

//  Serial.begin(115200); 
  
//  while (!Serial){
//  }
//  Serial.print(F("\nStarting SAMD_MultipleRandomServos on ")); Serial.println(BOARD_NAME);
//  Serial.println(SAMD_ISR_SERVO_VERSION);
  SAMD_ISR_Servos.useTimer(USING_SAMD_TIMER);
  
  for (int index = 0; index < NUM_SERVOS; index++)
  {
    ISR_servo[index].servoIndex = SAMD_ISR_Servos.setupServo(ISR_servo[index].servoPin, MIN_MICROS, MAX_MICROS);

    if (ISR_servo[index].servoIndex != -1)
    {
//      Serial.print(F("Setup OK Servo index = ")); Serial.println(ISR_servo[index].servoIndex);
    }
    else
    {
//      Serial.print(F("Setup Failed Servo index = ")); Serial.println(ISR_servo[index].servoIndex);
    }
  }

  SAMD_ISR_Servos.setReadyToRun();
  SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 90);
  SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 90 );
  

  delay(200);

  // We start by connecting to a WiFi network

//  Serial.println();
//  Serial.println();
//  Serial.print("Connecting to ");
//  Serial.println(ssid);

  WiFi.begin(ssid,pass);
  tft.drawString("Connecting WiFi", 30, 140);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
//    Serial.print("Connecting to ");
//    Serial.println(ssid);
    WiFi.begin(ssid);
  }
  tft.setTextSize(2);
  tft.fillScreen(TFT_WHITE);
  tft.drawString("WiFi connected", 30, 100);

//  Serial.println("");
//  Serial.println("WiFi connected.");
//  Serial.println("IP address: ");
//  Serial.println(WiFi.localIP());
//  Serial.println(WiFi.macAddress());
//  Serial.println("Disabling Serial Connection");
//  Serial.end();
  server.begin();
  
  String ipString = WiFi.localIP().toString().c_str();
  tft.drawString(ipString, 30, 120);
  tft.drawString("Ready for Connection", 30, 140);    
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
  WiFiClient client = server.available(); // listen for incoming clients

   if (client) {
    boolean currLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        header += c;        
        if (c == '\n' && currLineIsBlank) 
        {        

          if (header.indexOf("stop") != -1)
          {
            SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 90 );
            SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 90 );
            
          }
          
          else if (header.indexOf("train") != -1)
          {
            sensorData[0] = gas.getGM102B();
            sensorData[1] = gas.getGM302B();
            sensorData[2] = gas.getGM502B();
            sensorData[3] = gas.getGM702B();
            int currClass = header[11] - '0';
            myKNN.addExample(sensorData,currClass);
            message = "Your data has been recorded for Class ";
            message += header[11];
            message += "\n";
          }

          else if (header.indexOf("classify") != -1)
          {
            sensorData[0] = gas.getGM102B();
            sensorData[1] = gas.getGM302B();
            sensorData[2] = gas.getGM502B();
            sensorData[3] = gas.getGM702B();
            classification = myKNN.classify(sensorData, num_Neighbors);
            message = "My guess is Class: ";
            message += classification;
            message += "\n";
          }
          else if (header.indexOf("reset") != -1)
          {
            KNNClassifier myKNN(num_Inputs);
            message = "Your training data has been deleted \n";
              
          }
          else if (header.indexOf("message") != -1)
          {
            client.println(message);
            
          }
          else if (header.indexOf("forward") != -1)
          {
            SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 0 );
            SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 180 );
           
          }
           

          else if (header.indexOf("left") != -1)
          {
            SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 0 );
            SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 0 );


          }

          else if (header.indexOf("right") != -1)
          {
            SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 180 );
            SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 180 );
          }  

          else if (header.indexOf("back") != -1)
          {
            SAMD_ISR_Servos.setPosition(ISR_servo[0].servoIndex, 180 );
            SAMD_ISR_Servos.setPosition(ISR_servo[1].servoIndex, 0 );

          }

          else 
          {
            HTML_webpage();
            client.print(webPage);
          }  
//          Serial.print(header);
     
          header = "";
          break;
        }
        if (c =='\n') currLineIsBlank = true;
        else if(c != '\r') currLineIsBlank = false;
      }
    }
    client.stop();
  }
}

void HTML_webpage(){
  webPage = "HTTP/1.1 200 OK \n Content-Type:text/html \n \n";
  webPage += R"***(
  <!DOCTYPE HTML>
  <head><meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <html>
  <body style = "background-color:lightslategrey;font-family: Helvetica, sans-serif">
      <h1 style="text-align:center;color:white">Hedgehog Remote Control</h1>
      <table style="margin-left:auto;margin-right:auto;margin-top:auto;margin-bottom:auto;">
          <tr><td colspan="3" align="center"><button style="height:100px;width:150px" onmousedown="toggleCheckbox('forward');" ontouchstart="toggleCheckbox('forward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Forward</button></td></tr>
          <tr><td align="center"><button style="height:100px;width:150px" onmousedown="toggleCheckbox('left');" ontouchstart="toggleCheckbox('left');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Left</button></td><td align="center"><button style="height:100px;width:100px" onmousedown="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('stop');">Stop</button></td><td align="center"><button style="height:100px;width:150px" onmousedown="toggleCheckbox('right');" ontouchstart="toggleCheckbox('right');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Right</button></td></tr>
          <tr><td colspan="3" align="center"><button style="height:100px;width:150px" onmousedown="toggleCheckbox('backward');" ontouchstart="toggleCheckbox('backward');" onmouseup="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop');">Backward</button></td></tr>
      </table>
      <br>      
      <br><br>
      <table style="margin-left:auto;margin-right:auto;margin-top:auto;margin-bottom:auto;">
      <tr><td colspan="3" align="center"><label for="class_num" style="text-align:center;color:white">Choose a Class:</label><select name="class_num" id="class_num"><option value="1">Class 1</option><option value="2">Class 2</option><option value="3">Class 3</option><option value="4">Class 4</option><option value="5">Class 5</option></select> 
      <tr><td colspan="3" align="center"><button style="height:100px;width:150px" onmousedown="buttonDownTrain('');" ontouchstart="buttonDownTrain('');">Train</button><button style="height:100px;width:150px" onmousedown="buttonDownClassify('');" ontouchstart="buttonDownClassify('');">Classify</button><button style="height:100px;width:150px" onmousedown="buttonDownReset('');" ontouchstart="buttonDownReset('');">Reset Training</button></td></tr>
      </table>
      <h3 style="text-align:center;color:white"><span id = "outputText" >Start training the hedgehog</span></h3> 
  <script>    
      function toggleCheckbox(x) 
      {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/" + x, true);
          xhr.send();
      }
      function buttonDownTrain() 
      {
          var select = document.getElementById('class_num');
          var value = select.options[select.selectedIndex].value;
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/train/"+ value, true);
          xhr.send();
          getMessage();
      }
      function buttonDownClassify() 
      {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/classify", true);
          xhr.send();
          getMessage();
      }
      function buttonDownReset() 
      {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/reset", true);
          xhr.send();
          getMessage();
      }
      function getMessage() 
      {
          var messageReq = new XMLHttpRequest();
          messageReq.onreadystatechange = function()
          {
              if(this.readyState == 4 && this.status == 200)
              {
                  document.getElementById("outputText").innerHTML = this.responseText;
              }
          };
          messageReq.open("GET","/message",true);
          messageReq.send(); 
      }
          
  </script>
  </body>
  </html>
  )***";
}
