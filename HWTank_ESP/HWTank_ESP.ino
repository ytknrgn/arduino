#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define wifi_ssid "Main_nomap_optout"
#define wifi_password "l2nIp8lP*hQjU^qY"
#define mqtt_server "10.10.2.1"
#define mqtt_client "HWTankESP"
#define temperature_topic "HWTank/Temperature"
#define wakeup_topic "HWTank/WakeUp"
#define ONE_WIRE_BUS D6 

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature TempSensors(&oneWire);

unsigned long startMillis;  
const unsigned long period = 60000; 

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  TempSensors.begin();
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
    TempSensors.requestTemperatures();
    float temp1 = TempSensors.getTempCByIndex(0);
    float temp2 = TempSensors.getTempCByIndex(1);
    float temp3 = TempSensors.getTempCByIndex(2);
    float temp4 = TempSensors.getTempCByIndex(3);
    float temp_avg = (temp1 + temp2 + temp3 + temp4)/4;
    
    if ((temp_avg > 0) && (temp_avg < 100)) {
      Serial.print("Temperature: ");
      Serial.println(String(temp_avg).c_str());
      client.publish(temperature_topic, String(temp_avg).c_str(), true);
    }
    
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
