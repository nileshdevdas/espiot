#include <PubSubClient.h>
#include <ESP8266WiFi.h>


WiFiClient espClient;
PubSubClient client(espClient);

String ssid = "nileshhp";
String password = "admin@123";

void callback(char* topic, byte* payload, unsigned int length) {  
  Serial.println("Received Message on Topic "); 
  Serial.println(" Message Contents ### "); 
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}


void setup() {
  delay(2000); 
  Serial.begin(115200);
  Serial.println("#########################################################################");
  Serial.println("Starting the Configuration Setup");
  Serial.print("Using SSID ");
  Serial.println(ssid); 
  Serial.print("Using Password " ); 
  Serial.println(password);
  Serial.println("#########################################################################");

  WiFi.begin(ssid, password); 
  WiFi.hostname("nileshdevice");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }
  Serial.println("WiFi Connected Successfully "); 
  WiFi.printDiag(Serial);
  Serial.println("#########################Connecting to HiveMQ broker###########################");
  
  client.setServer("broker.hivemq.com", 1883);

  String clientId = "NileshClient-";
  clientId += String(random(0xffff), HEX);

  if(client.connect(clientId.c_str())){
    Serial.println("Connected to Broker ############################");
     client.setCallback(callback);
     client.subscribe("NTTOPIC");
    Serial.println("Subscribing ");
    
    client.publish("NTTOPIC" , "on");
    Serial.println("Published ");
    
     delay(5000);
     client.publish("NTTOPIC", "off");
  }  
}

void loop() {
  // put your main code here, to run repeatedly:
    delay(5000);
    client.loop();
 }
