#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Servo.h> 

// Watson IoT connection details
#define MQTT_HOST "XXX.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:XXX:YYY:ZZZ"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "PPP"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_INTERVAL "iot-2/cmd/interval/fmt/json"

// Add WiFi connection information
char ssid[] = "cloud-iot-ufrj";  // your network SSID (name)
char pass[] = "ufrj-ibm-cloud";  // your network password

// Declarations concerning 
Servo myservo;     // create servo object to control a servo
//int xpin = A0;     // analog pin used to connect the Accelerometer
//int pos;           // variable to read the value from the analog pin</p>

// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

// variables to hold data
StaticJsonDocument<100> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
StaticJsonDocument<100> jsonReceiveDoc;
static char msg[50];

int16_t angle;

///////////////////////////////////// FUNÇÕES ////////////////////////////////////////////

void callback(char* topic, byte* payload, unsigned int length)
{
   // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  
  payload[length] = 0; 
  Serial.println((char *)payload);
  DeserializationError err = deserializeJson(jsonReceiveDoc, (char *)payload);
  if (err) 
  {
    Serial.print(F("deserializeJson() failed with code ")); 
    Serial.println(err.c_str());
  } 
  else 
  {
    JsonObject cmdData = jsonReceiveDoc.as<JsonObject>();
    if (0 == strcmp(topic, MQTT_TOPIC_INTERVAL)) 
    {
      //valid message received
      angle = cmdData["Angle_X"].as<int16_t>(); // this form allows you specify the type of the data you want from the JSON object
      Serial.print("Angle has been changed:");
      Serial.println(angle);
      jsonReceiveDoc.clear();
    } 
    else {Serial.println("Unknown command received");}
  }
}

//marrom gnd
//laranja 3v3
//amarelo d4

///////////////////////////////////// SETUP ////////////////////////////////////////////

void setup() 
{
  // Start serial console
  myservo.attach(2);  // D4
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while (!Serial) { }
  Serial.println();
  Serial.println("ESP8266 Sensor Application");

  // Start WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  
  // Connect to MQTT - IBM Watson IoT Platform
  while(! mqtt.connected())
  {
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) 
    { 
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_INTERVAL);
    } 
    else 
    {
      Serial.println("MQTT Failed to connect! ... retrying");
      delay(500);
    }
  }
}

///////////////////////////////////// LOOP ////////////////////////////////////////////

void loop() 
{
  mqtt.loop();
  while (!mqtt.connected()) 
  {
    Serial.print("Attempting MQTT connection...");    
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) 
    {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_INTERVAL);
      mqtt.loop();
    } 
    else 
    {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }

  myservo.write(abs(floor(angle))); 
  delay(1000);

  // Print Message to console in JSON format
  status["angle"] = angle;
  serializeJson(jsonDoc, msg, 50);
  Serial.println(msg);
  if (!mqtt.publish(MQTT_TOPIC, msg)) 
  {
    Serial.println("MQTT Publish failed");
  }

  Serial.print("Angle :");
  Serial.print(angle);
  Serial.println();
  
  for (int i = 0; i < 10; i++) 
  {
    mqtt.loop();
    delay(1000);
  }
}
