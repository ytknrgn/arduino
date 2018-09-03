#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "Main_nomap_optout"
#define wifi_password "l2nIp8lP*hQjU^qY"
#define mqtt_server "10.10.2.1"
#define mqtt_client "DrivewayESP"
#define alarm_topic "Driveway/Alarm"
#define wakeup_topic "Driveway/WakeUp"

WiFiClient espClient;
PubSubClient client(espClient);

int drivewayPin = D6;
int drivewayStatePrev = LOW; 
long lastDebounceTimeDriveway = 0;  
long debounceDelayDriveway = 50; 
unsigned long startMillis;  
const unsigned long period = 30000; 

void setup() {
  Serial.begin(115200);
  pinMode(drivewayPin, INPUT_PULLUP);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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
  Serial.println("WiFi connected");
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
  
  int drivewayStateNew = digitalRead(drivewayPin); 
  if ( (millis() - lastDebounceTimeDriveway) > debounceDelayDriveway) {
    if ((drivewayStateNew == HIGH) && (drivewayStatePrev == LOW))  {            
      Serial.println("Driveway triggered!");
      lastDebounceTimeDriveway = millis();
      client.publish(alarm_topic, "1", true);  
    } 
    else if ((drivewayStateNew == LOW) && (drivewayStatePrev == HIGH)) {
      Serial.println("Driveway trigger ended.");
      lastDebounceTimeDriveway = millis();
      client.publish(alarm_topic, "0", true);
    }
    drivewayStatePrev = drivewayStateNew;
  }

  if (millis() - startMillis >= period) { 
    Serial.println("Sending wake up message now...");
    client.publish(wakeup_topic, "ON", true);
    startMillis = millis();  
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect(mqtt_client)) {
      Serial.println("connected");  
    } else {
      Serial.print("MQTT connection failed, code:");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
