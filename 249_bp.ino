#include<WiFi.h>
#include<Adafruit_MQTT.h>
#include<Adafruit_MQTT_Client.h>
#include<DHT.h>

//  rgb led details
byte rpin = 25;
byte gpin = 26;
byte bpin = 27;
byte rchannel = 0;
byte gchannel = 1;
byte bchannel = 2;
byte resolution = 8;
int frequency = 5000;

//  dht details
byte dht_pin = 4;
#define dht_type DHT11
DHT dht(dht_pin , dht_type);

//  wifi credentials
const char ssid[] = "write your ssid";
const char password[] = "write your password";

//  io details
#define IO_USERNAME  "write your username"
#define IO_KEY       "write your key"
#define IO_BROKER    "io.adafruit.com"
#define IO_PORT       1883

//  client details
WiFiClient wificlient;
Adafruit_MQTT_Client mqtt(&wificlient , IO_BROKER , IO_PORT , IO_USERNAME , IO_KEY);

/*  
  Create feed object to be subscribed 
  SYNTAX FOR REFERENCE : 
  */
  Adafruit_MQTT_Subscribe FEED OBJECT = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/redValue");
  Adafruit_MQTT_Subscribe FEED OBJECT = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/greenValue");
  Adafruit_MQTT_Subscribe FEED OBJECT = Adafruit_MQTT_Subscribe(&mqtt , IO_USERNAME"/feeds/blueValue");
/*  
  create feed objects to publish data
  SYNTAX FOR REFERENCE : 
  */
  Adafruit_MQTT_Publish feed object = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/humidity");  
  Adafruit_MQTT_Publish feed object = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/temperature(*C)");
  Adafruit_MQTT_Publish feed object = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/temperature(*F)");
  Adafruit_MQTT_Publish feed object = Adafruit_MQTT_Publish(&mqtt , IO_USERNAME"/feeds/temperature(*K)");
 
void setup()
{
  Serial.begin(115200);

  //  connecting with wifi
  Serial.print("Connecting with : ");
  Serial.println(ssid);
  WiFi.begin(ssid , password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected !");
  Serial.print("IP assigned by AP : ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //  RGB led setup
  ledcSetup(rchannel , frequency , resolution);
  ledcSetup(gchannel , frequency , resolution);
  ledcSetup(bchannel , frequency , resolution);

  //  attaching pins with channel
  ledcAttachPin(rpin , rchannel);
  ledcAttachPin(gpin , gchannel);
  ledcAttachPin(bpin , bchannel);

  //  dht setup
  dht.begin();

  //  feeds to be subscribed
  mqtt.subscribe(&red);
  mqtt.subscribe(&green);
  mqtt.subscribe(&blue);
  
}

void loop()
{
  //  connecting with server
  mqttconnect();

  //  reading values from dht sensor
  float tempc = dht.readTemperature();
  float tempf = dht.readTemperature(true);
  float tempk = tempc + 273.15;
  float humidity = dht.readHumidity();
  float dew_point = (tempc - (100 - humidity) / 5);  //  dew point in celcius

  if (isnan(tempc)  ||  isnan(tempf)  ||  isnan(humidity))
  {
    Serial.println("Sensor not working!");
    delay(1000);
    return;
  }

  //  printing these values on serial monitor
  String val = String(tempc) + " *C" + "\t" + String(tempf) + " *F" + "\t" + String(tempk) + " *K" + "\t" + 
               String(humidity) + " %RH" + "\t" + String(dew_point) + " *C";
  Serial.println(val);
  

  //  publish the DHT data to the feeds
  Adafruit_MQTT_Subscribe *subscription;
  while(true){
    subscription = mqtt.readSubscription(5000);
    if(subscription == &red){
      string r = (char *)red.lastread;
      makecolor(r,g,b);
    }
    if(subscription == &green){
      string g = (char *)green.lastread;
      makecolor(r,g,b);
    }
    if(subscription == &blue){
      string b = (char *)blue.lastread;
      makecolor(r,g,b);
    }
  }
  
  

  //  subscribe the slider values to control RGB
  
  
}

void mqttconnect()
{
  //  if already connected, return
  if (mqtt.connected())return;

  //  if not, connect
  else
  {
    while (true)
    {
      int connection = mqtt.connect();  //  mqq client has all details of client, port , username, key
      if (connection  ==  0)
      {
        Serial.println("Connected to IO");
        break;  //  connected
      }
      else
      {
        Serial.println("Can't Connect");
        mqtt.disconnect();
        Serial.println(mqtt.connectErrorString(connection));  //  printing error message
        delay(5000);  //  wait for 5 seconds
      }
    }
  }


  //  wait for some time
  delay(5000);
}

void makecolor(byte r , byte g , byte b)
{
  //  printing values
  Serial.print("RED : ");
  Serial.print(r);
  Serial.print('\t');
  Serial.print("GREEN : ");
  Serial.print(g);
  Serial.print('\t');
  Serial.print("BLUE : ");
  Serial.println(b);

  //  writing values
  ledcWrite(rchannel , r);
  ledcWrite(gchannel , g);
  ledcWrite(bchannel , b);
}
