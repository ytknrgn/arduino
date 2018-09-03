#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define wifi_ssid "Main_nomap_optout"
#define wifi_password "l2nIp8lP*hQjU^qY"
#define mqtt_server "10.10.2.1"
#define mqtt_client "CarESP"
#define wakeup_topic "Car/WakeUp"

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  Serial.println("Sending wake up message now...");
  client.publish(wakeup_topic, "ON", true);
  delay(100);
  Serial.println("Going into deep sleep for 30 seconds...");
  ESP.deepSleep(30e6); // 30 seconds
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
