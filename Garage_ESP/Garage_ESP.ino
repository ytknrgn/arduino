#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define wifi_ssid "Main_nomap_optout"
#define wifi_password "l2nIp8lP*hQjU^qY"
#define mqtt_server "10.10.2.1"
#define mqtt_client "GarageESP"
#define motion_topic "Garage/Motion"
#define door_int_topic "Garage/DoorInt"
#define door_ext_topic "Garage/DoorExt"
#define humidity_topic "Garage/Humidity"
#define temperature_topic "Garage/Temperature"
#define pressure_topic "Garage/Pressure" 
#define wakeup_topic "Garage/WakeUp"

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BME280 bme;
 
int doorIntPin = D6;
int doorIntStatePrev = HIGH; 
long doorIntLastDebounceTime = 0; 
int doorExtPin = D7;
int doorExtStatePrev = HIGH; 
long doorExtLastDebounceTime = 0;  
long debounceDelay = 50; 

int motionPin = D8;
int motionStatePrev = HIGH; 
long motionStart = 0;  
long motionDelay = 15000;

long wakeUpStart;  
long wakeUpPeriod = 60000;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(motionPin, INPUT_PULLUP);
  pinMode(doorIntPin, INPUT_PULLUP);  
  pinMode(doorExtPin, INPUT_PULLUP); 
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //setup_bme();
  wakeUpStart = millis(); 
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_bme() {
  bool status;
  status = bme.begin();
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      while (1);
  }
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
    Adafruit_BME280::SAMPLING_X1, // Temperature oversampling
    Adafruit_BME280::SAMPLING_X1, // Pressure oversampling
    Adafruit_BME280::SAMPLING_X1, // Humidity oversampling
    Adafruit_BME280::FILTER_OFF,
    Adafruit_BME280::STANDBY_MS_1000);
  Serial.println();
  delay(100); // let sensor boot up
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(10);
    setup_wifi();
    return;
  }
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  int doorIntStateNew = digitalRead(doorIntPin); 
  if ( (millis() - doorIntLastDebounceTime) > debounceDelay) {
    if ((doorIntStateNew == HIGH) && (doorIntStatePrev == LOW)) {   
        Serial.println("Internal garage door opened!");
        client.publish(door_int_topic, "1", true);
        doorIntLastDebounceTime = millis();
      } 
    else if ((doorIntStateNew == LOW) && (doorIntStatePrev == HIGH)) {
        Serial.println("Internal garage door closed.");
        client.publish(door_int_topic, "0", true);
        doorIntLastDebounceTime = millis();
      }
    doorIntStatePrev = doorIntStateNew;
  }

  int doorExtStateNew = digitalRead(doorExtPin); 
  if ( (millis() - doorExtLastDebounceTime) > debounceDelay) {
    if ((doorExtStateNew == HIGH) && (doorExtStatePrev == LOW)) {   
        Serial.println("External garage door opened!");
        client.publish(door_ext_topic, "1", true);
        doorExtLastDebounceTime = millis();
      } 
    else if ((doorExtStateNew == LOW) && (doorExtStatePrev == HIGH)) {
        Serial.println("External garage door closed.");
        client.publish(door_ext_topic, "0", true);
        doorExtLastDebounceTime = millis();
      }
    doorExtStatePrev = doorExtStateNew;
  }
  
  int motionStateNew = digitalRead(motionPin); 
  if ( (millis() - motionStart) > motionDelay) {
    if ((motionStateNew == HIGH) && (motionStatePrev == LOW))  {            
      Serial.println("Motion triggered!");
      motionStart = millis();
      client.publish(motion_topic, "1", true);  
    } 
    else if ((motionStateNew == LOW) && (motionStatePrev == HIGH)) {
      Serial.println("Motion trigger ended.");
      motionStart = millis();
      client.publish(motion_topic, "0", true);
    }
    motionStatePrev = motionStateNew;
  }

  if (millis() - wakeUpStart >= wakeUpPeriod) {
    Serial.println("Sending wake up message now...");
    client.publish(wakeup_topic, "ON", true);
    Serial.println("Wake up message sent.");
    
    wakeUpStart = millis();
    }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(mqtt_client)) {
      Serial.println("MQTT connected.");  
    } else {
      Serial.print("MQTT connection failed, code:");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
