#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"

MPU6050 accelgyro;

// Watson IoT connection details
#define MQTT_HOST "XXX.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:XXX:YYY:ZZZ"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "PPP"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/display/fmt/json"

#define ACL6050_ACCEL_OFFSET_X -4678
#define ACL6050_ACCEL_OFFSET_Y -1907
#define ACL6050_ACCEL_OFFSET_Z 3922
//#define ACL6050_GYRO_OFFSET_X  178
//#define ACL6050_GYRO_OFFSET_Y  -128
//#define ACL6050_GYRO_OFFSET_Z  -12

// Add WiFi connection information
char ssid[] = "cloud-iot-ufrj";  // your network SSID (name)
char pass[] = "ufrj-ibm-cloud";  // your network password

int16_t ax, ay, az;
//int16_t gx, gy, gz;
//int16_t x,y,z;
//int16_t xang, yang, zang;

void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

void callback(char* topic, byte* payload, unsigned int length)
{
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  
  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.println((char *)payload);
}

// variables to hold data
StaticJsonDocument<100> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[100];

///////////////////////////////////// SETUP ////////////////////////////////////////////

void setup()
{
  // Start serial console
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while (!Serial) {}
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
  
  // Start connected devices
  // join I2C bus (I2Cdev library doesn't do this automatically)
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
  #endif
  Serial.println("MCU6050 initializing...");
  accelgyro.initialize();
  
  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  accelgyro.setXAccelOffset(ACL6050_ACCEL_OFFSET_X);
  accelgyro.setYAccelOffset(ACL6050_ACCEL_OFFSET_Y);
  accelgyro.setZAccelOffset(ACL6050_ACCEL_OFFSET_Z);
  //accelgyro.setXGyroOffset(ACL6050_GYRO_OFFSET_X);
  //accelgyro.setYGyroOffset(ACL6050_GYRO_OFFSET_Y);
  //accelgyro.setZGyroOffset(ACL6050_GYRO_OFFSET_Z);

  // Connect to MQTT - IBM Watson IoT Platform
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN))
  {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);
  } 
  else 
  {
    Serial.println("MQTT Failed to connect!");
    ESP.reset();
  }
}

///////////////////////////////////// LOOP ////////////////////////////////////////////

void loop()
{  
  delay(100);

  mqtt.loop();
  while (!mqtt.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) 
    {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.loop();
    } 
    else 
    {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }

  //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  accelgyro.getAcceleration(&ax, &ay, &az);
  //accelgyro.getRotation(&gx, &gy, &gz);

//  x = ax/16384.;
//  y = ay/16384.;
//  z = az/16384.;

//  xang = (180/3.141592) * atan(x / sqrt(sq(y) + sq(z))); 
//  yang = (180/3.141592) * atan(y / sqrt(sq(x) + sq(z)));
//  zang = (180/3.141592) * atan(sqrt(sq(y) + sq(x)) / z);
 
  //Print Message to console in JSON format
  status["acel_x"] = ax/16384.;
  status["acel_y"] = ay/16384.;
  status["acel_z"] = az/16384.;
//  status["angle_x"] = xang;
//  status["angle_y"] = yang;
//  status["angle_z"] = zang;
  serializeJson(jsonDoc, msg, 100); //Existe um limite para msg? É possível enviar duas msg?

  Serial.print("a/g:\t");
  Serial.print(ax/16384.); Serial.print("\t");
  Serial.print(ay/16384.); Serial.print("\t");
  Serial.print(az/16384.); Serial.print("\t");
  //Serial.print(gx/131); Serial.print("\t");
  //Serial.print(gy/131); Serial.print("\t");
  //Serial.println(gz/131);
   
  Serial.println(msg);
  
  if (!mqtt.publish(MQTT_TOPIC, msg)) 
  {
    Serial.println("MQTT Publish failed");
  }
  //Pause - but keep polling MQTT for incoming messages
  for (int i = 0; i < 10; i++) 
  {
    mqtt.loop();
    delay(500);
  }
}
