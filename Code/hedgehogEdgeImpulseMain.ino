#include <Hedgehog_inferencing.h>
#include <rpcWiFi.h>
#include <Multichannel_Gas_GMXXX.h>
#include <Wire.h>
#include "SAMD_ISR_Servo.h"
#include "TFT_eSPI.h"



#define ISR_SERVO_DEBUG             1
#define USING_SAMD_TIMER            TIMER_TC3
#define MIN_MICROS        500
#define MAX_MICROS        2500 

#define SERVO_PIN_1       A5
#define SERVO_PIN_2       A3

TFT_eSPI tft;
GAS_GMXXX<TwoWire> gas;



String message = "Ready for Classification";
float features[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];
static bool debug_nn = false; 


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



const char *ssid = "YourWiFiSSID";
const char *pass = "YourWiFiPass"; 
//Servo lServo;
//Servo rServo;
int pos = 0;


float no2 = 0;
float c2h5ch = 0;
float voc = 0;
float co = 0;

WiFiServer server(80);

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

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
          
          else if (header.indexOf("classify") != -1)
          {
            for(int i=0;i<EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE;i=i+4){
      
              features[i]=gas.getGM102B();  
              features[i+1] = gas.getGM302B();
              features[i+2]=gas.getGM502B();
              features[i+3]=gas.getGM702B();
      
              delay(EI_CLASSIFIER_INTERVAL_MS);
            }          
            ei_impulse_result_t result = { 0 };
            signal_t signal;
            numpy::signal_from_buffer(features, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);
            String maxlabel;
            float maxval = 0;
            run_classifier(&signal, &result, debug_nn);
            for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
              ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
              if(result.classification[ix].value > maxval){
                maxval = result.classification[ix].value;
                maxlabel = result.classification[ix].label;
              }
             }
            message = "My guess is Class: " + maxlabel + " " + maxval;
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
      <tr><td colspan="3" align="center"><button style="height:100px;width:150px" onmousedown="buttonDownClassify('');" ontouchstart="buttonDownClassify('');">Classify</button></td></tr>
      </table>
      <h3 style="text-align:center;color:white"><span id = "outputText" >Ready for Classification</span></h3> 
  <script>    
      function toggleCheckbox(x) 
      {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/" + x, true);
          xhr.send();
      }

      function buttonDownClassify() 
      {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/classify", true);
          xhr.send();
          setTimeout(getMessage(),1000);         
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
