#include <OneWire.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DallasTemperature.h>

///////////////////////////////////////////
/////// NODE CONFIGURATION /////////////////
// uncomment correct define

// LOCATION
//#define KITCHEN
//#define LIVING_ROOM
#define WARDROBE

// SENSOR TYPES
//#define DHT_11
#define DHT_22

// END OF NODE CONFIGURATION //////////////


///////////////////////////////////////////
// NETWORK CONFIGURATION //////////////////
#define wifi_ssid "Gaik-home"
#define wifi_password "fourwordsalluppercase"

#define mqtt_server "192.168.1.24"
#define mqtt_user "owntracks"
#define mqtt_password "Knyfelek"


///////////////////////////////////////////
// MQTT TOPICS DEFINITON //////////////////

#if defined(KITCHEN)
  #define tHumidity "sensor/kitchenHumidity"
  #define tTempIn "sensor/kitchenTempIn"
  #define tTempOut "sensor/kitchenTempOut"
#elif defined(LIVING_ROOM)
  #define tHumidity "sensor/livingroomHumidity"
  #define tTempIn "sensor/livingroomTempIn"
  #define tTempOut "sensor/livingroomTempOut"
#elif defined(WARDROBE)
  #define tHumidity "sensor/wardrobeHumidity"
  #define tTempIn "sensor/wardrobeTempIn"
  #define tTempOut "sensor/wardrobeTempOut"
#endif


///////////////////////////////////////////
// HW CONFIG //////////////////////////////
#if defined(DHT_11)
  #define DHTTYPE DHT11
#elif defined(DHT_22)
  #define DHTTYPE DHT22
#endif

#define DHTPIN 2
#define ONE_WIRE_BUS 4

///////////////////////////////////////////
// OTHER DEFINITIONS
#if defined(DHT_11)
  #define minDiffTempDHT 1.0
  #define minDiffHumDHT 4.0
#elif defined(DHT_22)
  #define minDiffTempDHT 0.2
  #define minDiffHumDHT 1.5
#endif

#define minDiffTempDS 0.2

///////////////////////////////////////////
// FUNCTIONS DECLARATIONS ////////////////
void setup_wifi();
void reconnect();
bool hasValueChanged(float, float, float);


//////////////////////////////////////////
// GLOBALS AND STARTING VALUES //////////
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 15);
OneWire onewire(ONE_WIRE_BUS);
DallasTemperature sensors(&onewire);

long lastMsg = 0;
float inTemp = 0.0;
float inHumidity = 0.0;
float outTemp = 0.0;

///////////////////////////////////////////
// SYSTEM SETUP //////////////////////////
void setup() {
  Serial.begin(115200);
  dht.begin();
  sensors.begin();
  setup_wifi();
  client.setServer(mqtt_server, 8883);
}

///////////////////////////////////////////
// MAIN LOOP //////////////////////////
void loop() {
  Serial.println("hsdgfkjs");
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
    
    // read inside tempratature
    if (hasValueChanged(newTemp, inTemp, minDiffTempDHT)) {
      inTemp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(inTemp).c_str());
      client.publish(tTempIn, String(inTemp).c_str(), true);
    }

    // read inside humidity
    if (hasValueChanged(newHum, inHumidity, minDiffHumDHT)) {
      inHumidity = newHum;
      Serial.print("New humidity:");
      Serial.println(String(inHumidity).c_str());
      client.publish(tHumidity, String(inHumidity).c_str(), true);
    }

    // read outside temperature
    if (hasValueChanged(new_outtemp, outTemp, minDiffTempDS)) {
      outTemp = new_outtemp;
      Serial.print("Outside temperature:");
      Serial.println(outTemp);
      client.publish(tTempOut, String(outTemp).c_str(), true);
    }
  }
}

/////////////////////////////////////
// FUNCTIONS DEFINITIONS ////////////
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

bool hasValueChanged(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
    (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}
