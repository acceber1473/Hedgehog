//Wifi Bot for Chris Hedgehog Project
//Used WifiWebServer example and randomnerdtutorials.com for inspiration.



#include <Arduino.h>
#include <SPI.h>
#include <WiFiNINA.h>
#include <Servo.h>

Servo lServo;
Servo rServo;
String header;

char ssid[] = "";        // your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;          

int status = WL_IDLE_STATUS;      


WiFiServer server(80);

void setup() {
  lServo.attach(1);
  rServo.attach(0);
  //Initialize serial and wait for port to open:
  
  //Serial.begin(9600);
  // while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    //Serial.print("Attempting to connect to SSID: ");
    //Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  //printWifiStatus();
}


void loop() {
  // listen for incoming clients
  WiFiClient client = server.available();
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

            if (header.indexOf("GET /forward") != -1){
              lServo.write(135);
              rServo.write(45);
            }
            if (header.indexOf("GET /stop") != -1){
              lServo.write(90);
              rServo.write(90);
            }
            if (header.indexOf("GET /right") != -1){
              lServo.write(135);
              rServo.write(135);
            }
            if (header.indexOf("GET /left") != -1){
              lServo.write(45);
              rServo.write(45);
            }
            if (header.indexOf("GET /back") != -1){
              lServo.write(45);
              rServo.write(135);
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
    //Serial.println("client disconnected");
    //Serial.println("");
}
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
