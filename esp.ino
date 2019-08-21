#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "FS.h";

ESP8266WebServer server(80); // starting the webserver 80

String ssid = "nileshhome12";
String password = "admin@123";

String deviceid = "nileshhome"; // also using as the default ap name

String web_on_html =         "<html><head> \
                                 <style> \
                                      .buttonred { \
                                      background-color: red; \
                                      border: none; \
                                      color: white; \
                                      padding: 15px 32px; \
                                      text-align: center; \
                                      text-decoration: none; \
                                      display: inline-block; \
                                      font-size: 16px; \
                                    } \
                                  </style> \
                                 </head> \
                                 <body> \
                                 <h1>SONOFF switch is ON</h1> \
                                  <p><a href=\"off\"> <button class='buttonred'>OFF</button></a>\
                                  </body> \
                                 </html> ";



String web_off_html = "<html><head> \
                                 <style> \
                                      .buttongreen{ \
                                      background-color: green; \
                                      border: none; \
                                      color: white; \
                                      padding: 15px 32px; \
                                      text-align: center; \
                                      text-decoration: none; \
                                      display: inline-block; \
                                      font-size: 16px; \
                                    } \
                                  </style> \
                                 </head> \
                                 <body> \
                                 <h1>SONOFF switch is OFF</h1> \
                                  <p><a href=\"on\"> <button class='buttongreen'>ON</button></a>\
                                  </body> \
                                 </html> ";







void setup() {
  Serial.begin(115200); // serialdebug 
  Serial.print("\n\n ************************************Program Started ************************************ \n\n");
  delay(1000);
  bool result = SPIFFS.begin();
  Serial.println("  SPIFFS opened: " + result);
  File f = SPIFFS.open("/config.ini", "r");
  if (!f) {
    Serial.println("*************** No Config File Found Running in AP Mode *************** ");
    APMode();
  } else {
    Serial.println("*************** Config File Found Running in Client Mode *************** ");
    connectWifi();
  }
}

void connectWifi() {
  Serial.println("########### connecting wifi ############");
  File config = SPIFFS.open("/config.ini", "r");
  Serial.println("########### Opened config.ini ############");
  while (config.available()) {
    String str = config.readStringUntil('\n');
    char str_array[str.length()];
    str.toCharArray(str_array, str.length());
    char *ptr = strtok(str_array, "=");
    if (strcmp(ptr, "ssid") == 0) {
      Serial.println("######################## SSID ########################");
      ssid = strtok(NULL, "=");
      Serial.print(ssid);
    }
    if (strcmp(ptr, "password") == 0) {
      Serial.println("######################## PASSWORD ########################");
      password = strtok(NULL, "=");
      Serial.print(password);
    }

    if (strcmp(ptr, "deviceid") == 0) {
      Serial.println("######################## PASSWORD ########################");
      deviceid = strtok(NULL, "=");
      Serial.print(deviceid);
    }
  }
  Serial.println(" ######################### Connecting to Wifi using #############################" );
  Serial.println(ssid);
  Serial.println(password);
  Serial.println(deviceid);
  char char_array[deviceid.length()];
  deviceid.toCharArray(char_array, deviceid.length() + 1);
  wifi_station_set_hostname(char_array);
  bool hostname  = WiFi.hostname(char_array);
  Serial.println(" -------------------------------- HOSTNAME ----------------------------------------");
  Serial.println(deviceid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  if (MDNS.begin(deviceid)) {
      Serial.println("MDNS started");
  }
  Serial.println("-----------------------------------------------");
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
  WiFi.printDiag(Serial);
  server.on("/reset", handleReset);
  server.on("/on", []() {
    server.send(200, "text/html", web_on_html);
    pinMode(0, OUTPUT);
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    digitalWrite(0, HIGH);
    delay(1000);
  });
  server.on("/off", []() {
    server.send(200, "text/html", web_off_html);
    pinMode(0, OUTPUT);
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);
    digitalWrite(0, LOW);
    delay(1000);
  });
  server.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP server started");
  Serial.println("----------------------------------------------");
}


void APMode() {
  WiFi.hostname(deviceid);
  WiFi.softAP(deviceid);
  if (MDNS.begin(deviceid)) {
      Serial.println("MDNS started");
  }

  Serial.println("################## IOTAPI ################");
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on("/config", handleConfig);
  server.on("/reset", handleReset);
  server.begin();
  Serial.println("HTTP server started");
}
void loop() {
  MDNS.update();
  // put your main code here, to run repeatedly:
  server.handleClient();
}

void handleRoot() {
  server.send(200, "text/html", "<html><body> \
                                 <h1>Please choose configuration</h1> \
                                 <form method='post' action='config'> \
                                 <table> \
                                 <tr><td><label>Device Name</label></td><td><input type='text' name='deviceid'></td></tr> \
                                 <tr><td><label>SSID</label></td><td><input type='text' name='ssid'></td></tr> \
                                 <tr><td><label>Password</label></td><td><input type='password' name='password'></td></tr> \
                                 <tr><td colspan='2'><input type='submit' value='Configure'><td></tr> \
                                 </form> \
                                 </body></html> ");
}


void handleReset() {
  SPIFFS.format();
  server.send(200, "text/html", "<h1>Config Reset Done Restart </h1>");
  delay(2000);
  ESP.restart();
}

void handleConfig() {
  Serial.println("Received Config Request");
  Serial.print(server.args());
    File f = SPIFFS.open("/config.ini", "r");
  if (!f) {
    File config = SPIFFS.open("/config.ini", "w");
    Serial.print("Config File Opened -> ");
    Serial.print(f);
    if (config) {
      for (int i = 0; i < server.args(); i++) {
        if (server.argName(i) == "ssid") {
          Serial.print("SSID ");
          Serial.print(server.arg(i));
          config.print("ssid=");
          config.println(server.arg(i));
        }
        if (server.argName(i) == "password") {
          Serial.print("Password ");
          Serial.print(server.arg(i));
          config.print("password=");
          config.println(server.arg(i));
        }
        if (server.argName(i) == "deviceid") {
          Serial.print("Password ");
          Serial.print(server.arg(i));
          config.print("deviceid=");
          config.println(server.arg(i));
        }
      }
      config.close();
      server.send(200, "text/html" , "<h2 style='color:green;'>Config Successfully Done Restarting in Client mode</h2>");
      ESP.restart();
    } else {
      server.send(200, "text/html" , "<h2 style='color:red'>Configuration Error Occured Please Try Again</h2>");
    }
  }
}