#include <WiFiClient.h>
#include <ESP32WebServer.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "FS.h"
#include <SD.h>
#include <SPI.h>

const char* ssid = "ssid";
const char* password = "pass";

ESP32WebServer server(80);
File root;

void handleRoot() {
  root = SD.open("/index.html");
  if (root) {  
    /* respond the content of file to client by calling streamFile()*/
    size_t sent = server.streamFile(root, "text/html");
    /* close the file */
    root.close();
  } else {
    Serial.println("error opening index");
  }
}

bool loadFromSDCARD(String path){
  path.toLowerCase();
  Serial.println(path);
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "/index.html";
  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".txt")) dataType = "text/plain";
  else if(path.endsWith(".zip")) dataType = "application/zip";  
  if(path == "/favicon.ico")
    return false;
  
  root = SD.open((String("/") + path).c_str());
  if (!root){
    Serial.println("failed to open file");
    return false;
  }

  if (server.streamFile(root, dataType) != root.size()) {
    Serial.println("Sent less data than expected!");
  }

  root.close();
  return true;
}

void handleNotFound(){
  if(loadFromSDCARD(server.uri())) return;
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}

void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  //use IP or iotsharing.local to access webserver
  if (MDNS.begin("iotsharing")) {
    Serial.println("MDNS responder started");
  }
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  //handle uri  
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
}
