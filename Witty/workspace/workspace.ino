#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

///////////////////////////////////////////
// NETWORK CONFIGURATION //////////////////
#define wifi_ssid "Gaik-home"
#define wifi_password "fourwordsalluppercase"

#define mqtt_server "192.168.1.24"
#define mqtt_user "owntracks"
#define mqtt_password "Knyfelek"

///////////////////////////////////////////
// MQTT TOPICS DEFINITON //////////////////

#define tKitchenHumidity "sensor/kitchenHumidity"
#define tKitchenTempIn "sensor/kitchenTempIn"
#define tKitchenTempOut "sensor/kitchenTempOut"

///////////////////////////////////////////
// HW CONFIG //////////////////////////////
#define DHTTYPE DHT11
#define DHTPIN 4
#define ONE_WIRE_BUS 2

///////////////////////////////////////////
// OTHER DEFINITIONS
#define minDiffTempDHT ????
#define minDiffTempDS 0.5
#define minDiffHumDHT ????


///////////////////////////////////////////
// INITIAL SETUP //////////////////////////
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 15);
OneWire onewire(ONE_WIRE_BUS);
DallasTemperature sensors(&onewire);

void setup() {
  Serial.begin(115200);
  dht.begin();
  sensors.begin();
  setup_wifi();
  client.setServer(mqtt_server, 8883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;
float outtemp = 0.0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;


    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();
    sensors.requestTemperatures();
    delay(500);
    float new_outtemp = sensors.getTempCByIndex(0);
    
    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      client.publish(tKitchenTempIn, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, diff)) {
      hum = newHum;
      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      client.publish(tKitchenHumidity, String(hum).c_str(), true);
    }
    if (checkBound(new_outtemp, outtemp, diff)) {
      outtemp = new_outtemp;
      Serial.print("Outside temperature:");
      Serial.println(outtemp);
      client.publish(tKitchenTempOut, String(outtemp).c_str(), true);
    }
  }
}