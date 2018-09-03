#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define wifi_ssid "Main_nomap_optout"
#define wifi_password "l2nIp8lP*hQjU^qY"
#define mqtt_server "10.10.2.1"
#define mqtt_client "ShedESP"
#define lux_topic "Shed/Lux"
#define humidity_topic "Shed/Humidity"
#define temperature_topic "Shed/Temperature"
#define pressure_topic "Shed/Pressure" 
#define wakeup_topic "Shed/WakeUp"

WiFiClient espClient;
PubSubClient client(espClient);

Adafruit_BME280 bme;
BH1750 lightMeter;

unsigned long startMillis;  
const unsigned long period = 60000; 

void setup() {
  Serial.begin(115200);
  Wire.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  lightMeter.begin();
  setup_bme();
  startMillis = millis(); 
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

  if (millis() - startMillis >= period) { 
    uint16_t lux = lightMeter.readLightLevel();
    Serial.print("Lux: ");
    Serial.println(String(lux).c_str());
    client.publish(lux_topic, String(lux).c_str(), true);
  
    bme.takeForcedMeasurement();
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure()/100;
  
    Serial.print("Temperature: ");
    Serial.println(String(temp).c_str());
    client.publish(temperature_topic, String(temp).c_str(), true);
      
    Serial.print("Humidity: ");
    Serial.println(String(hum).c_str());
    client.publish(humidity_topic, String(hum).c_str(), true);
      
    Serial.print("Pressure: ");
    Serial.println(String(pres).c_str());
    client.publish(pressure_topic, String(pres).c_str(), true); 
  
    Serial.println("Sending wake up message now...");
    client.publish(wakeup_topic, "ON", true);
    Serial.println("Wake up message sent.");
    
    startMillis = millis(); 
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
